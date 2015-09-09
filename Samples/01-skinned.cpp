#include <sample.hpp>
#include <glitter.hpp>

#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"
#include "mesh.hpp"
#include "debugrender.hpp"

extern void printMatrix(glm::mat4 const & m);
extern void printVector(glm::vec4 const & v);
extern void printVector(glm::vec3 const & v);
extern void printQuat(glm::quat const & q);
// END Sample Code

struct SkinnedMeshSampleContext {
  Shader skinnedShader;
  Shader floorShader;
  Mesh* cubeMesh;
  Mesh* floorMesh;
  Mesh* skinnedMesh;
};

static void*
loadSample() {
  SkinnedMeshSampleContext* context = new SkinnedMeshSampleContext();

  context->skinnedShader.attach("skinned.vert");
  context->skinnedShader.attach("diffuse.frag");
  context->skinnedShader.link();

  context->floorShader.attach("diffuse.vert");
  context->floorShader.attach("diffuse.frag");
  context->floorShader.link();

  float fw = 50;
  float fh = 50;
  float fx = 0;
  float fy = 0;

  std::vector<GLuint> indices;
  std::vector<Vertex> vertices;
  Vertex tl, tr, bl, br;
  tl.position = glm::vec3(-(fw * 0.5) + fx, 0.0f,  (fh * 0.5) + fy);
  tr.position = glm::vec3( (fw * 0.5) + fx, 0.0f,  (fh * 0.5) + fy);
  bl.position = glm::vec3(-(fw * 0.5) + fx, 0.0f, -(fh * 0.5) + fy);
  br.position = glm::vec3( (fw * 0.5) + fx, 0.0f, -(fh * 0.5) + fy);
  tl.uv = glm::vec2(0.0, 0.0);
  tr.uv = glm::vec2(1.0, 0.0);
  bl.uv = glm::vec2(0.0, 1.0);
  br.uv = glm::vec2(1.0, 1.0);
  tl.color = glm::vec4(1.0, 1.0, 1.0, 1.0);
  tr.color = glm::vec4(1.0, 1.0, 1.0, 1.0);
  bl.color = glm::vec4(1.0, 1.0, 1.0, 1.0);
  br.color = glm::vec4(1.0, 1.0, 1.0, 1.0);
  tl.normal = glm::vec3(0.0, 1.0, 0.0);
  tr.normal = glm::vec3(0.0, 1.0, 0.0);
  bl.normal = glm::vec3(0.0, 1.0, 0.0);
  br.normal = glm::vec3(0.0, 1.0, 0.0);

  vertices.push_back(tl);
  vertices.push_back(tr);
  vertices.push_back(bl);
  vertices.push_back(br);

  indices.push_back(0); indices.push_back(1); indices.push_back(2);
  indices.push_back(2); indices.push_back(1); indices.push_back(3);

  context->floorMesh = new Mesh(vertices, indices, {}, {}, {}, nullptr);
  context->cubeMesh = new Mesh("Cube.dae");
  //context->skinnedMesh = new Mesh("SkinnedMesh_TestA.dae");
  context->skinnedMesh = new Mesh("SkinnedMesh_TestB.dae");

  DbgInit();

  return context;
}

static void
unloadSample(void* ctx) {
  SkinnedMeshSampleContext* context = (SkinnedMeshSampleContext*) ctx;

  DbgShutdown();

  delete context->cubeMesh;
  delete context->floorMesh;
  delete context->skinnedMesh;
  delete context;
}

static void
drawSample(void* ctx) {
  SkinnedMeshSampleContext* context = (SkinnedMeshSampleContext*) ctx;

  DbgBegin();

  ImGui::ShowTestWindow(nullptr);

  static float floorColor[] = { 0.2058824f, 0.2058824f, 0.2058824f, 1.0f };
  static float skinColor[] = { 0.2058824f, 0.2058824f, 0.2058824f, 1.0f };
  static float lightDir[] = { 0.815f, 0.482f, -0.317f };
  static float lightColor[] = { 0.933f, 0.803f, 0.686f, 1.0f };
  static float rotAmt[] = { 0.0f, 0.0f, 0.0f };
  static float fov = 65.0f;
  static bool floorShown = true;
  static bool debugRendererEnabled = true;


  ImGui::SetWindowPos("BoneHierarchy Display", ImVec2(0,30));

  const float editWidth = 350;
  ImGui::SetNextWindowPos(ImVec2(mWidth - editWidth - 20, 30), ImGuiSetCond_Once);
  ImGui::SetNextWindowContentWidth(editWidth);

  static bool floorSettingsOpen = true;
  if (ImGui::Begin("Render", &floorSettingsOpen, ImVec2(0,0), 0.3f, ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoSavedSettings)) {
    ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth());
    if (ImGui::Button("Reload Mesh Shader")) {
      context->skinnedShader.attach("skinned.vert");
      context->skinnedShader.attach("diffuse.frag");
      context->skinnedShader.link();
    } // ImGui::SameLine();
    ImGui::PopItemWidth();

    ImGui::ColorEdit3("Floor Color", floorColor);
    ImGui::ColorEdit3("Mesh Color", skinColor);
    ImGui::ColorEdit3("Light Color", lightColor);
    ImGui::DragFloat3("Light Dir", lightDir, 0.01, -1.0, 1.0, nullptr, 1.0);
    ImGui::DragFloat3("Cam Rotation", rotAmt, 0.01, -M_PI, M_PI, nullptr, 1.0);
    ImGui::DragFloat("Field of View", &fov, 1, -360, 360, nullptr, 1.0);
    ImGui::Checkbox("Show Floor Plane", &floorShown);
    ImGui::Checkbox("Enable Debug Render", &debugRendererEnabled);

    auto nameGetter = [](void* data, int index, const char** name) {
      auto mesh = (Mesh *)data;
      *name = mesh->getAnimationLabels()[index].c_str();

      return true;
    };

    int animIndex = context->skinnedMesh->getAnimationIndex();
    if (ImGui::Combo("Animation", &animIndex, nameGetter, (void *) context->skinnedMesh, context->skinnedMesh->getAnimationLabels().size(), 4))
      context->skinnedMesh->setAnimationIndex(animIndex);
  }
  ImGui::End();

  DbgEnable(debugRendererEnabled);

  //rotAmt += M_PI_2 * 0.01;
  glm::mat4 model;
  glm::vec3 camPos = glm::vec3(0.0f, 25.0f, 35.0f);
  glm::mat4 camRot = glm::rotate(glm::mat4(), rotAmt[2], glm::vec3(0, 0, 1)) * glm::rotate(glm::mat4(), rotAmt[1], glm::vec3(0, 1, 0)) * glm::rotate(glm::mat4(), rotAmt[0], glm::vec3(1, 0, 0));

  glm::vec4 camPosRot = camRot * glm::vec4(camPos, 1);
//      glm::mat4 view = glm::lookAt(camPos.xyz(), glm::vec3(), glm::vec3(0.0f, 1.0f, 0.0f));
  glm::mat4 view = glm::lookAt(camPosRot.xyz(), glm::vec3(), glm::vec3(0.0f, 1.0f, 0.0f));
  glm::mat4 projection = glm::perspective(glm::radians(fov), (float)mWidth / (float)mHeight, 0.01f, 100.0f);

  DbgSetView(view);
  DbgSetProjection(projection);

//    floorShader.activate();
//
//    floorShader.bind("uModel", model);
//    floorShader.bind("uView", view);
//    floorShader.bind("uProj", projection);
//
//    floorShader.bind("uColor", glm::make_vec4(floorColor));
//    floorShader.bind("uLightDir", glm::normalize(glm::make_vec3(lightDir)));
//    floorShader.bind("uLightColor", glm::make_vec4(lightColor));
//    floorShader.bind("uPointLightPosition", glm::vec3(-3.52f, 3.3f, -0.82f));
//    floorShader.bind("uPointLightColor", glm::vec4(0.149f, 0.304f, 0.433f, 1.0f));
//
//    if (floorShown)
//    {
//      floorMesh.draw(floorShader);
//      DbgDrawCoordSystem(model);
//    }

//      floorShader.bind("uModel", glm::scale(model, glm::vec3(5)));
//      cube.draw(floorShader);

  context->skinnedShader.activate();

  //shader.bind("uModel", glm::translate(glm::mat4(), glm::vec3(0, 5, 0)));
  context->skinnedShader.bind("uView", view);
  context->skinnedShader.bind("uProj", projection);

  context->skinnedShader.bind("uColor", glm::make_vec4(skinColor));
  context->skinnedShader.bind("uLightDir", glm::normalize(glm::make_vec3(lightDir)));
  context->skinnedShader.bind("uLightColor", glm::make_vec4(lightColor));
  context->skinnedShader.bind("uPointLightPosition", glm::vec3(-3.52f, 3.3f, -0.82f));
  context->skinnedShader.bind("uPointLightColor", glm::vec4(0.149f, 0.304f, 0.433f, 1.0f));

  //mesh.draw(shader, glm::scale(model, glm::vec3(5)));
  context->skinnedMesh->draw(context->skinnedShader, model);

  DbgEnd();

  // END Sample Code
}

static Sample gSkinnedSample("Skinned Mesh Rendering", loadSample, unloadSample, drawSample);