#include <glitter.hpp>

#include "debugrender.hpp"
#include "shader.hpp"
#include <glm/gtx/orthonormalize.hpp>

using namespace glm;

static glm::mat4 gView;
static glm::mat4 gProj;
static bool gEnabled;

static Shader* gDebugShader;
static std::vector<DebugEntry> gEntries;
static std::vector<DebugVertex> gVertices;
static std::vector<GLuint> gIndices;
static GLuint gVertexArray;
static GLuint gVertexBuffer;
static GLuint gElementBuffer;

struct DebugVertex
{
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec3 color;

  DebugVertex(glm::vec3 pos, glm::vec3 col = glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3 norm = glm::vec3())
    : position(pos)
    , normal(norm)
    , color(col)
  {
  }
};

struct DebugEntry
{
  glm::mat4 view;
  glm::mat4 proj;
  glm::mat4 model;

  GLsizei numElements;
  GLenum primType;
};

void DbgSetView(glm::mat4 value) { gView = value; }
void DbgSetProjection(glm::mat4 value) { gProj = value; }
void DbgEnable(bool enable) { gEnabled = enable; }


void DbgInit()
{
  DbgEnable(true);

  glGenVertexArrays(1, &gVertexArray);
  glBindVertexArray(gVertexArray);

  glGenBuffers(1, & gVertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, gVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

  // Copy Index Buffer Data
  glGenBuffers(1, & gElementBuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gElementBuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (GLvoid *) offsetof(DebugVertex, position));
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (GLvoid *) offsetof(DebugVertex, normal));
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (GLvoid *) offsetof(DebugVertex, color));
  glEnableVertexAttribArray(0); // Vertex Positions
  glEnableVertexAttribArray(1); // Vertex Normal
  glEnableVertexAttribArray(2); // Vertex Colors

  glBindVertexArray(0);

  gDebugShader  = new Shader();
  gDebugShader->attach("debug.vert");
  gDebugShader->attach("debug.frag");
  gDebugShader->link();
}

void DbgShutdown()
{
  glDeleteBuffers(1, &gVertexBuffer);
  glDeleteBuffers(1, &gElementBuffer);
  glDeleteVertexArrays(1, &gVertexArray);

  delete gDebugShader;
  gDebugShader = nullptr;
}

void DbgBegin()
{
  gVertices.clear();
  gIndices.clear();
  gEntries.clear();
}

void DbgEnd()
{
  if (!gEntries.size()) return;

  glDisable(GL_DEPTH_TEST);
  glDepthMask(false);

  glBindVertexArray(gVertexArray);
  //glBindBuffer(GL_ARRAY_BUFFER, gVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER,
               gVertices.size() * sizeof(DebugVertex),
               gVertices.data(), GL_DYNAMIC_DRAW);
  //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gElementBuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               gIndices.size() * sizeof(GLuint),
               gIndices.data(), GL_DYNAMIC_DRAW);

  gDebugShader->activate();

  for (auto& entry : gEntries)
  {
    gDebugShader->bind("uView", entry.view);
    gDebugShader->bind("uProj", entry.proj);
    gDebugShader->bind("uModel", entry.model);
    glDrawElements(entry.primType, entry.numElements, GL_UNSIGNED_INT, nullptr);
  }

  glBindVertexArray(0);

  glDepthMask(true);
  glEnable(GL_DEPTH_TEST);
}

DebugEntry& DbgPushEntry()
{
  DebugEntry entry = {};
  entry.model = mat4();
  entry.view = gView;
  entry.proj = gProj;
  entry.primType = GL_LINES;
  gEntries.push_back(entry);

  DebugEntry& result = gEntries.data()[gEntries.size() - 1];
  return result;
}

static GLsizei
PushLine(DebugEntry& entry, vec3 from, vec3 to, vec3 color)
{
  assert(entry.primType == GL_LINES);

  GLuint base = gVertices.size();

  gVertices.push_back(DebugVertex(from, color));
  gVertices.push_back(DebugVertex(to, color));

  gIndices.push_back(base + 0); gIndices.push_back(base + 1);
  entry.numElements += 2;
}

void DbgDrawLine(vec3 from, vec3 to, vec3 color)
{
  if (gEnabled)
  {
    auto &entry = DbgPushEntry();
    PushLine(entry, from, to, color);
  }
}

void DbgDrawCoordSystem(mat4 model, float lineLength)
{
  if (gEnabled)
  {
    auto &entry = DbgPushEntry();

#if 0
    model[0][3] = 0;
    model[1][3] = 0;
    model[2][3] = 0;
    //model[3] = vec4(0, 0, 0, 1);

    model[0] = normalize(model[0]);
    model[1] = normalize(model[1]);
    model[2] = normalize(model[2]);
    entry.model = model;
#else
    auto rot = mat3(model);
    rot = orthonormalize(rot);

    entry.model = mat4(rot);
    entry.model[3] = model[3];
#endif

    PushLine(entry, vec3(0, 0, 0), vec3(lineLength, 0, 0), vec3(1, 0, 0));
    PushLine(entry, vec3(0, 0, 0), vec3(0, lineLength, 0), vec3(0, 1, 0));
    PushLine(entry, vec3(0, 0, 0), vec3(0, 0, lineLength), vec3(0, 0, 1));
  }
}