#pragma once

// System Headers
#include <assimp/importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glad/glad.h>
#define GLM_SWIZZLE
#include <glm/glm.hpp>

// Standard Headers
#include <map>
#include <memory>
#include <vector>
#include <chrono>

// Define Namespace
//namespace Mirage
//{
    // Vertex Format
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec4 color;
        glm::vec4 boneWeights;
        glm::ivec4 boneIndices;
        glm::vec2 uv;
    };

    struct BoneInfo {
      glm::mat4 boneOffset;
      glm::mat4 finalTransformation;
      std::string name;
    };

    class Shader;

    class Mesh
    {
    public:

      // Implement Default Constructor and Destructor
       Mesh() : mDepth(0), mParent(nullptr), mAnimIndex(0), mStartTime(std::chrono::high_resolution_clock::now()) { glGenVertexArrays(1, & mVertexArray); }
      ~Mesh() { glDeleteVertexArrays(1, & mVertexArray); }

      // Implement Custom Constructors
      Mesh(std::string const & filename);
      Mesh(std::vector<Vertex> const & vertices,
           std::vector<GLuint> const & indices,
           std::map<GLuint, std::string> const & textures,
           std::map<std::string, uint> boneMapping,
           std::vector<BoneInfo> bones,
           aiScene const * scene,
           aiNode const * node = nullptr,
           Mesh* parent = nullptr,
           std::string name = "",
           int depth = 0);

      // Public Member Functions
      void draw(Shader const & shader, glm::mat4 model = glm::mat4());
      const std::vector<std::string> const & getAnimationLabels() { return mAnimList; }
      int getAnimationIndex() const { return mAnimIndex; }
      void setAnimationIndex(int value) { mAnimIndex = value; }

    private:

      // Disable Copying and Assignment
      Mesh(Mesh const &) = delete;
      Mesh & operator=(Mesh const &) = delete;

      // Private Member Functions
      void parse(std::string const & path, aiScene const * scene, aiNode const * node);
      void parse(std::string const & path, aiScene const * scene, aiNode const * node, aiMesh const * mesh);
      void parse(std::string const & path, aiScene const * scene, aiNode const * node, aiAnimation const * anim);
      void loadBones(aiMesh const * mesh, std::vector<Vertex> & vertices, std::map<std::string, uint> & boneMapping, std::vector<BoneInfo> & bones);
      void transformBones(float time, std::vector<glm::mat4> & bones);
      void updateBoneHierarchy(float time, aiAnimation const * animation, aiNode const * node, glm::mat4 const & parent, int depth = 0);
      std::map<GLuint, std::string> process(std::string const & path,
                                            aiMaterial * material,
                                            aiTextureType type);

      // Private Member Containers
      std::vector<std::unique_ptr<Mesh>> mSubMeshes;
      std::vector<GLuint> mIndices;
      std::vector<Vertex> mVertices;
      std::map<GLuint, std::string> mTextures;
      std::string mName;

      // Private Member Variables
      GLuint mVertexArray;
      GLuint mVertexBuffer;
      GLuint mElementBuffer;
      GLuint mBoneTexture;
      int mDepth;
      aiScene const * mScene;

      bool mSceneOwned;

      typedef std::map<std::string, uint> BoneMap;

      std::vector<BoneInfo> mBoneInfo;
      BoneMap mBoneMapping;

      glm::mat4 mTransform;
      glm::mat4 mGlobalInverseTransform;

      Mesh* mParent;
      std::vector<std::string> mAnimList;
      std::chrono::time_point<std::chrono::high_resolution_clock> mStartTime;
      int mAnimIndex;
    };
//};
