

// Local Headers
#include "mesh.hpp"
#include "shader.hpp"
#include "debugrender.hpp"
#include <ImGui.h>

// System Headers
#include <stb_image.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <sstream>
#include <glm/gtx/string_cast.hpp>

// Define Namespace
//namespace Mirage
//{

  void
  printMatrix(glm::mat4 const & x)
  {
    printf("{\n"
           "  %g, %g, %g, %g,\n"
           "  %g, %g, %g, %g,\n"
           "  %g, %g, %g, %g,\n"
           "  %g, %g, %g, %g\n"
           "}\n",
           x[0][0], x[0][1], x[0][2], x[0][3],
           x[1][0], x[1][1], x[1][2], x[1][3],
           x[2][0], x[2][1], x[2][2], x[2][3],
           x[3][0], x[3][1], x[3][2], x[3][3]);
  }

  void
  printVector(glm::vec4 const & v)
  {
    printf("{ %g, %g, %g, %g }\n",
           v[0], v[1], v[2], v[3]);
  }

  void
  printVector(glm::ivec4 const & v)
  {
    printf("{ %d, %d, %d, %d }\n",
           v[0], v[1], v[2], v[3]);
  }

  void
  printVector(glm::vec3 const & v)
  {
    printf("{ %g, %g, %g }\n",
           v[0], v[1], v[2]);
  }

  void
  printQuat(glm::quat const & q)
  {
    printf("{ %g, %g, %g, %g }\n",
           q[0], q[1], q[2], q[3]);
  }

  void
  displayMatrix(std::string const & label, glm::mat4 const & x)
  {
    ImGui::Text("%s: {\n"
      "  %g, %g, %g, %g,\n"
      "  %g, %g, %g, %g,\n"
      "  %g, %g, %g, %g,\n"
      "  %g, %g, %g, %g\n"
      "}\n", label.data(),
      x[0][0], x[0][1], x[0][2], x[0][3],
      x[1][0], x[1][1], x[1][2], x[1][3],
      x[2][0], x[2][1], x[2][2], x[2][3],
      x[3][0], x[3][1], x[3][2], x[3][3]);
  }

  void
  displayVec(std::string const & label, glm::vec4 const & m)
  {
    ImGui::Text("%s: { %g, %g, %g, %g }\n",
      label.data(), m[0], m[1], m[2], m[3]);
  }

  void
  displayVec(std::string const & label, glm::vec3 const & m)
  {
    ImGui::Text("%s: { %g, %g, %g }\n",
      label.data(), m[0], m[1], m[2]);
  }

  void
  displayQuat(std::string const & label, glm::quat const & m)
  {
    auto axis = glm::axis(m);
    auto angle = glm::angle(m);
    ImGui::Text("%s:\n  angle: %g\n  axis: { %g, %g, %g }", label.data(), angle, axis.x, axis.y, axis.z);
  }

  static glm::quat
  make_quat(aiQuaternion const & v) { return glm::make_quat(&v.x); }
  static glm::vec3
  make_vec(aiVector3D const & v) { return glm::make_vec3(&v.x); }
  static glm::vec2
  make_vec(aiVector2D const & v) { return glm::make_vec2(&v.x); }

  static glm::mat4
  make_mat(aiMatrix4x4 const & v)
  {
    glm::mat4 r;
#if 1
    r = glm::make_mat4(&v.a1);
    r = glm::transpose(r);
#else
    r[0][0] = v.a1; r[1][0] = v.a2;
    r[2][0] = v.a3; r[3][0] = v.a4;
    r[0][1] = v.b1; r[1][1] = v.b2;
    r[2][1] = v.b3; r[3][1] = v.b4;
    r[0][2] = v.c1; r[1][2] = v.c2;
    r[2][2] = v.c3; r[3][2] = v.c4;
    r[0][3] = v.d1; r[1][3] = v.d2;
    r[2][3] = v.d3; r[3][3] = v.d4;
#endif

    return r;
  }

  static glm::mat3
  make_mat(aiMatrix3x3 const & v)
  {
    glm::mat3 r;
#if 1
    r = glm::make_mat3(&v.a1);
    r = glm::transpose(r);
#else
    r[0][0] = v.a1;
    r[1][0] = v.a2;
    r[2][0] = v.a3;
    r[0][1] = v.b1;
    r[1][1] = v.b2;
    r[2][1] = v.b3;
    r[0][2] = v.c1;
    r[1][2] = v.c2;
    r[2][2] = v.c3;
#endif

    return r;
  }

  Mesh::Mesh(std::string const & filename) : Mesh()
  {
    printf("Mesh created with path\n");
    // Load a Model from File
    Assimp::Importer loader;
    if (loader.ReadFile(
      PROJECT_SOURCE_DIR "/Glitter/Models/" + filename,
//        aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs
      aiProcessPreset_TargetRealtime_MaxQuality |
      aiProcess_OptimizeGraph                   |
      aiProcess_FlipUVs
      ))
    {
      // HACKS: Need to let the importer dealloc this...
      mScene = std::shared_ptr<aiScene>(loader.GetOrphanedScene());
      mTransform = make_mat(mScene->mRootNode->mTransformation);

      // Walk the Tree of Scene Nodes
      auto index = filename.find_last_of("/");
      if (!mScene) fprintf(stderr, "%s\n", loader.GetErrorString());
      else
      {
        parse(filename.substr(0, index), mScene, mScene->mRootNode);
        for (unsigned int i = 0; i < mScene->mNumAnimations; i++)
          parse(filename.substr(0, index), mScene, mScene->mRootNode, mScene->mAnimations[i]);
      }
    }
    else
    {
      fprintf(stderr, loader.GetErrorString());
    }
  }

  Mesh::Mesh(std::vector<Vertex> const & vertices,
             std::vector<GLuint> const & indices,
             std::map<GLuint, std::string> const & textures,
             std::map<std::string, uint> boneMapping,
             std::vector<BoneInfo> bones,
             aiNode const * node,
             Mesh* parent,
             std::string name,
             int depth)
                  : mIndices(indices)
                  , mVertices(vertices)
                  , mTextures(textures)
                  , mBoneInfo(bones)
                  , mBoneMapping(boneMapping)
                  , mDepth(depth)
                  , mName(name)
                  , mParent(parent)
                  , mStartTime(std::chrono::high_resolution_clock::now())
  {
    if (!node && getScene())
      node = getScene()->mRootNode;

    mTransform = node ? make_mat(node->mTransformation) : glm::mat4();

    printf("Mesh created with data\n");
    // Bind a Vertex Array Object
    glGenVertexArrays(1, & mVertexArray);
    glBindVertexArray(mVertexArray);

    // Copy Vertex Buffer Data
    glGenBuffers(1, & mVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER,
                 mVertices.size() * sizeof(Vertex),
                 mVertices.data(), GL_STATIC_DRAW);

    // Copy Index Buffer Data
    glGenBuffers(1, & mElementBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 mIndices.size() * sizeof(GLuint),
                 mIndices.data(), GL_STATIC_DRAW);

    // Set Shader Attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *) offsetof(Vertex, position));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *) offsetof(Vertex, normal));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *) offsetof(Vertex, color));
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *) offsetof(Vertex, boneWeights));
    glVertexAttribIPointer(4, 4, GL_INT, sizeof(Vertex), (GLvoid *) offsetof(Vertex, boneIndices));
    glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *) offsetof(Vertex, uv));
    glEnableVertexAttribArray(0); // Vertex Positions
    glEnableVertexAttribArray(1); // Vertex Normals
    glEnableVertexAttribArray(2); // Vertex Colors
    glEnableVertexAttribArray(3); // Vertex BoneWeights
    glEnableVertexAttribArray(4); // Vertex BoneIndices
    glEnableVertexAttribArray(5); // Vertex UVs

    // Cleanup Buffers
    glBindVertexArray(0);
    glDeleteBuffers(1, & mVertexBuffer);
    glDeleteBuffers(1, & mElementBuffer);

    if (bones.size() > 0)
    {
      glGenTextures(1, &mBoneTexture);
      glBindTexture(GL_TEXTURE_2D, mBoneTexture);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 512, 512, 0, GL_RGBA, GL_FLOAT, 0);
    }
  }

  const aiNodeAnim * findNodeAnim(aiAnimation const * animation, std::string const & nodeName)
  {
    for (unsigned int i = 0; i < animation->mNumChannels; ++i)
    {
      auto channel = animation->mChannels[i];
      std::string channelName;
      if (channel->mNodeName.length != 0)
        channelName = channel->mNodeName.data;
      if (channelName == nodeName)
      {
        return channel;
      }
    }

    return nullptr;
  }

  aiVector3D calcInterpolatedScaling(float time, aiNodeAnim const * anim)
  {
    if (anim->mNumScalingKeys == 1) {
      return anim->mScalingKeys[0].mValue;
    }

    uint index = 0;
    for (uint i = 0; i < anim->mNumScalingKeys- 1; ++i)
    {
      if (time < (float)anim->mScalingKeys[i + 1].mTime)
      {
        index = i;
        break;
      }
    }

    uint nextIndex = index + 1;
    auto key0 = anim->mScalingKeys[index];
    auto key1 = anim->mScalingKeys[nextIndex];
    float deltaTime = key1.mTime - key0.mTime;
    aiVector3D deltaValue = key1.mValue - key0.mValue;
    float interpFactor = (time - key0.mTime) / deltaTime;

    return key0.mValue + deltaValue * interpFactor;
  }

  aiVector3D calcInterpolatedTranslation(float time, aiNodeAnim const * anim)
  {
    if (anim->mNumPositionKeys == 1) {
      return anim->mPositionKeys[0].mValue;
    }

    uint index = 0;
    for (uint i = 0; i < anim->mNumPositionKeys - 1; ++i)
    {
      if (time < (float)anim->mPositionKeys[i + 1].mTime)
      {
        index = i;
        break;
      }
    }

    uint nextIndex = index + 1;
    auto key0 = anim->mPositionKeys[index];
    auto key1 = anim->mPositionKeys[nextIndex];
    float deltaTime = key1.mTime - key0.mTime;
    aiVector3D deltaValue = key1.mValue - key0.mValue;
    float interpFactor = (time - key0.mTime) / deltaTime;

    return key0.mValue + deltaValue * interpFactor;
  }

  aiQuaternion calcInterpolatedRotation(float time, aiNodeAnim const * anim)
  {
    if (anim->mNumRotationKeys == 1) {
      return anim->mRotationKeys[0].mValue;
    }

    uint rotationIndex = 0;
    for (uint i = 0; i < anim->mNumRotationKeys - 1; ++i)
    {
      if (time < (float)anim->mRotationKeys[i + 1].mTime)
      {
        rotationIndex = i;
        break;
      }
    }

    uint nextRotationIndex = rotationIndex + 1;
    auto key0 = anim->mRotationKeys[rotationIndex];
    auto key1 = anim->mRotationKeys[nextRotationIndex];
    float deltaTime = key1.mTime - key0.mTime;
    float interpFactor = (time - key0.mTime) / deltaTime;

    aiQuaternion out;
    aiQuaternion::Interpolate(out, key0.mValue, key1.mValue, interpFactor);

    return out.Normalize();
  }

  void Mesh::updateBoneHierarchy(float time, aiAnimation const * animation, aiNode const * node, glm::mat4 const & parent, int depth)
  {
    std::string nodeName;
    if (node->mName.length != 0)
      nodeName = node->mName.data;

    glm::mat4 nodeTransform = make_mat(node->mTransformation);
    bool opened = depth >= 0 ? ImGui::TreeNode(node, "%s", nodeName.data()) : false;

    const aiNodeAnim * nodeAnim = findNodeAnim(animation, nodeName);
    if (nodeAnim)
    {
      aiVector3D scaling = calcInterpolatedScaling(time, nodeAnim);
      glm::mat4 scale = glm::scale(glm::mat4(), make_vec(scaling));

      aiQuaternion rotation = calcInterpolatedRotation(time, nodeAnim);
      glm::mat4 rot = glm::mat4(make_mat(rotation.GetMatrix()));

      aiVector3D translation = calcInterpolatedTranslation(time, nodeAnim);
      glm::mat4 trans = glm::translate(glm::mat4(), make_vec(translation));

      if (opened)
      {
        ImGui::Text("time: %g", time);
        displayVec("translation", make_vec(translation));
        displayMatrix("trans", trans);
        displayQuat("rotation", make_quat(rotation));
        displayMatrix("rot", rot);
        displayVec("scaling", make_vec(scaling));
        displayMatrix("scale", scale);
      }

#if 1
      nodeTransform = trans * rot * scale;
#else
      nodeTransform = scale * rot * trans;
#endif
    }

#if 1
    glm::mat4 globalTransform = parent * nodeTransform;
#else
    glm::mat4 globalTransform = nodeTransform * parent;
#endif

    if (opened)
    {
#if 1
      displayMatrix("parent", parent);
      displayMatrix("local", nodeTransform);
      displayMatrix("global", globalTransform);
#endif
    }

    auto bone = mBoneMapping.find(nodeName);
    if (bone != mBoneMapping.end())
    {
      auto boneIndex = bone->second;
      auto& boneInfo = mBoneInfo[boneIndex];

#if 1
      boneInfo.finalTransformation = mGlobalInverseTransform * globalTransform * boneInfo.boneOffset;
#else
      boneInfo.finalTransformation = boneInfo.boneOffset * globalTransform * mGlobalInverseTransform;
#endif
      if (opened)
      {
        displayMatrix("offset", boneInfo.boneOffset);
      }
      //boneInfo.finalTransformation = boneInfo.boneOffset * globalTransform;
      DbgDrawCoordSystem(globalTransform);
      //boneInfo.finalTransformation = mGlobalInverseTransform * boneInfo.boneOffset;
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i)
    {
      updateBoneHierarchy(time, animation, node->mChildren[i], globalTransform, opened ? depth + 1 : -1);
    }

    if (opened)
      ImGui::TreePop();
  }

  void Mesh::transformBones(float time, std::vector<glm::mat4> & bones)
  {
    // TODO: Do this at load time?
    int animIndex = 0;
    AnimationClip clip;

    if (mParent)
    {
      animIndex = mParent->mAnimIndex;
      clip = mParent->mAnimations[animIndex];
    }
    else
    {
      animIndex = mAnimIndex;
      clip = mAnimations[animIndex];
    }

    double ticksPerSecond =
        clip.animation->mTicksPerSecond > std::numeric_limits<double>::epsilon()
          ? clip.animation->mTicksPerSecond
          : 25.0f;
    double timeInTicks = time * ticksPerSecond;
#if 1
    double animTime = fmod(timeInTicks, clip.animation->mDuration);
#else
    double animTime = 0.625f;
#endif


    ImGui::Begin("BoneHierarchy Display", nullptr, ImVec2(30,30), 0.3f, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoSavedSettings);

    updateBoneHierarchy((float)animTime, clip.animation, getScene()->mRootNode, glm::mat4());

    bool opened = ImGui::TreeNode(this, "Final Bones: %s", mName.data());
    bones.resize(mBoneInfo.size());

    for (size_t i = 0; i < mBoneInfo.size(); ++i)
    {
      bones[i] = mBoneInfo[i].finalTransformation;
      if (opened)
      {
        auto boneIter = std::find_if(mBoneMapping.begin(), mBoneMapping.end(), [&i](BoneMap::value_type b) { return b.second == i; });

        if (boneIter != mBoneMapping.end())
        {
          displayMatrix(boneIter->first, bones[i]);
        }
        else
        {
          std::stringstream ss;
          ss << i;
          displayMatrix(ss.str(), bones[i]);
        }
      }
    }

    if (opened)
      ImGui::TreePop();

    ImGui::End();
  }

  void Mesh::draw(Shader const & shader, glm::mat4 model)
  {
    auto global = mTransform;
    unsigned int unit = 0, diffuse = 0, specular = 0;
    for (auto &i : mSubMeshes) i->draw(shader, model);
    if (mIndices.size() == 0) return;

    for (auto &i : mTextures)
    {   // Set Correct Uniform Names Using Texture Type (Omit ID for 0th Texture)
      std::string uniform = i.second;
           if (i.second == "Diffuse")  uniform += (diffuse++  > 0) ? std::to_string(diffuse)  : "";
      else if (i.second == "Specular") uniform += (specular++ > 0) ? std::to_string(specular) : "";

      // Bind Correct Textures and Vertex Array Before Drawing
      glActiveTexture(GL_TEXTURE0 + unit);
      glBindTexture(GL_TEXTURE_2D, i.first);
      shader.bind(std::string("u") + uniform, (int)++unit);
    }

    shader.bind("uModel", model);

    // TODO: Fill bones array from animation.
    if (mBoneInfo.size() > 0 && mBoneTexture > 0)
    {
      std::vector<glm::mat4> bones;
      auto endTime = std::chrono::high_resolution_clock::now();
#if 1
      auto t = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - mStartTime).count() / 1000.f;
#else
      auto t = 0.f;
#endif

      transformBones(t, bones);

      glActiveTexture(GL_TEXTURE0 + unit);
      glBindTexture(GL_TEXTURE_2D, mBoneTexture);

      // FIXME: Very bad hacky way to update this texture.
      size_t texSize = 512 * 512 * 4 * sizeof(float);
      size_t bonesSize = bones.size() * 16 * sizeof(float);
      void* texData = malloc(texSize);
      memset(texData, 0, texSize);
      memcpy(texData, glm::value_ptr(*bones.data()), bonesSize);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 512, 512, GL_RGBA, GL_FLOAT, texData);
      free(texData);

      shader.bind("uBoneTexture", (int)unit);
      ++unit;
    }

    glBindVertexArray(mVertexArray);
    glDrawElements(GL_TRIANGLES, (GLsizei) mIndices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
  }

  void Mesh::parse(std::string const & path, std::shared_ptr<aiScene> scene, aiNode const * node)
  {
    printf("node (%s): ", node->mName.data);
    printMatrix(make_mat(node->mTransformation));

    for (unsigned int i = 0; i < node->mNumMeshes; i++)
      parse(path, scene, node, scene->mMeshes[node->mMeshes[i]]);
    for (unsigned int i = 0; i < node->mNumChildren; i++)
      parse(path, scene, node->mChildren[i]);
  }

  void Mesh::parse(std::string const & path, std::shared_ptr<aiScene> scene, aiNode const * node, aiMesh const * mesh)
  {
    // Create Vertex Data from Mesh Node
    std::vector<Vertex> vertices; Vertex vertex;
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
      if (mesh->mTextureCoords[0])
        vertex.uv       = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
      vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
      vertex.normal   = glm::vec3(mesh->mNormals[i].x,  mesh->mNormals[i].y,  mesh->mNormals[i].z);
      if (mesh->mColors[0])
        vertex.color    = glm::vec4(mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b, mesh->mColors[0][i].a);
      vertex.boneIndices = glm::ivec4(0);
      vertex.boneWeights = glm::vec4(0.0f);
      vertices.push_back(vertex);
    }

    std::map<std::string, uint> boneMapping;
    std::vector<BoneInfo> bones;
    loadBones(mesh, vertices, boneMapping, bones);

    mGlobalInverseTransform = glm::inverse(make_mat(scene->mRootNode->mTransformation));
    //mGlobalInverseTransform = glm::inverse(mTransform);

    // Create Mesh Indices for Indexed Drawing
    std::vector<GLuint> indices;
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; j++)
    indices.push_back(mesh->mFaces[i].mIndices[j]);

    // Load Mesh Textures into VRAM
    std::map<GLuint, std::string> textures;
    auto diffuse  = process(path, scene->mMaterials[mesh->mMaterialIndex], aiTextureType_DIFFUSE);
    auto specular = process(path, scene->mMaterials[mesh->mMaterialIndex], aiTextureType_SPECULAR);
    textures.insert(diffuse.begin(), diffuse.end());
    textures.insert(specular.begin(), specular.end());

    // Create New Mesh Node

    mSubMeshes.push_back(std::unique_ptr<Mesh>(new Mesh(vertices, indices, textures, boneMapping, bones, node, this, std::string(mesh->mName.data), mDepth + 1)));
  }

  void Mesh::parse(std::string const & path, std::shared_ptr<aiScene> scene, aiNode const * node, aiAnimation const * animation)
  {
    // TODO: AnimationSet should own the scene.
    AnimationClip clip;
    clip.scene = scene;
    clip.animation = animation;
    clip.name = clip.animation->mName.data;

    auto end_index = path.find_last_of(".");
    auto begin_index = path.find("_");

    mAnimList.push_back(path.substr(begin_index + 1, end_index - (begin_index + 1)) + " (" + std::to_string(clip.animation->mDuration) + " sec)");
    mAnimations.push_back(clip);
  }

  void Mesh::loadBones(aiMesh const * mesh, std::vector<Vertex> & vertices, std::map<std::string, uint> & boneMapping, std::vector<BoneInfo> & bones)
  {
    for (uint i = 0; i < mesh->mNumBones; ++i)
    {
      uint boneIndex = 0;
      auto bone = mesh->mBones[i];
      std::string boneName(bone->mName.data);

      auto it = boneMapping.find(boneName);
      if (it == boneMapping.end())
      {
        boneIndex = boneMapping.size();
        BoneInfo bi;
        bi.name = boneName;
        bones.push_back(bi);
        bones[boneIndex].boneOffset = make_mat(bone->mOffsetMatrix);
        boneMapping[boneName] = boneIndex;

        printf("mesh %s: assigning bone %s to index %d\n", mesh->mName.C_Str(), boneName.c_str(), boneIndex);
      }
      else
      {
        boneIndex = it->second;
      }

      for (uint j = 0; j < bone->mNumWeights; ++j)
      {
        uint vertexId = bone->mWeights[j].mVertexId;
        float weight = bone->mWeights[j].mWeight;
        Vertex& vertex = vertices[vertexId];

//        printf("bone %d -> vertex %d (%g): pos: ", boneIndex, vertexId, weight);
//        printVector(vertex.position);

        for (uint k = 0; k < 4; ++k)
        {
          if (vertex.boneWeights[k] == 0.0)
          {
            vertex.boneIndices[k] = boneIndex;
            vertex.boneWeights[k] = weight;
            break;
          }
        }
      }
    }

    int vi = 0;
    for (auto& v : vertices)
    {
//      if (v.boneWeights[0] == 0.0f && v.boneWeights[1] == 0.0f)
      {
//        printf("boneWeights: %d", vi);
//        printVector(v.boneWeights);
        printf("boneIndices: %d", vi);
        printVector(v.boneIndices);
      }
      vi++;
    }
  }

  std::map<GLuint, std::string> Mesh::process(std::string const & path,
                                              aiMaterial * material,
                                              aiTextureType type)
  {
    std::map<GLuint, std::string> textures;
    for(unsigned int i = 0; i < material->GetTextureCount(type); i++)
    {
      // Define Some Local Variables
      GLenum format;
      GLuint texture;
      std::string mode;

      // Load the Texture Image from File
      aiString str; material->GetTexture(type, i, & str);
      std::string filename = str.C_Str(); int width, height, channels;
      filename = PROJECT_SOURCE_DIR "/Glitter/Models/" + path + "/" + filename;
      unsigned char * image = stbi_load(filename.c_str(), & width, & height, & channels, 0);
      if (!image) fprintf(stderr, "%s %s\n", "Failed to Load Texture", filename.c_str());

      // Set the Correct Channel Format
      switch (channels)
      {
          case 1 : format = GL_ALPHA;     break;
          case 2 : format = GL_LUMINANCE; break;
          case 3 : format = GL_RGB;       break;
          case 4 : format = GL_RGBA;      break;
      }

      // Bind Texture and Set Filtering Levels
      glGenTextures(1, & texture);
      glBindTexture(GL_TEXTURE_2D, texture);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexImage2D(GL_TEXTURE_2D, 0, format,
                   width, height, 0, format, GL_UNSIGNED_BYTE, image);
      glGenerateMipmap(GL_TEXTURE_2D);

      // Release Image Pointer and Store the Texture
      stbi_image_free(image);
           if (type == aiTextureType_DIFFUSE)  mode = "Diffuse";
      else if (type == aiTextureType_SPECULAR) mode = "Specular";
      textures.insert(std::make_pair(texture, mode));
    }   return textures;
  }

  void Mesh::loadAnimationClip(std::string const & filename)
  {
    Assimp::Importer loader;
    if (loader.ReadFile(
        PROJECT_SOURCE_DIR "/Glitter/Models/" + filename,
//        aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs
        aiProcessPreset_TargetRealtime_MaxQuality |
        aiProcess_OptimizeGraph                   |
        aiProcess_FlipUVs
    ))
    {
      // HACKS: Need to let the importer dealloc this...
      mScene = std::shared_ptr<aiScene>(loader.GetOrphanedScene());

      // Walk the Tree of Scene Nodes
      auto index = filename.find_last_of("/");
      auto file = filename.substr(0, index);
      if (!mScene) fprintf(stderr, "%s\n", loader.GetErrorString());
      else
      {
        for (unsigned int i = 0; i < mScene->mNumAnimations; i++)
          parse(file, mScene, mScene->mRootNode, mScene->mAnimations[i]);
      }
    }
    else
    {
      fprintf(stderr, loader.GetErrorString());
    }
  }
//};
