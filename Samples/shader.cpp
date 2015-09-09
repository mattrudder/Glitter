// Local Headers
#include "shader.hpp"

// Standard Headers
#include <cassert>
#include <fstream>
#include <memory>

// Define Namespace
//namespace Mirage
//{
    Shader & Shader::activate()
    {
        glUseProgram(mProgram);
        return *this;
    }

    void Shader::bind(unsigned int location, float value) const { glUniform1f(location, value); }
    void Shader::bind(unsigned int location, glm::mat4 const & matrix) const
    { glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix)); }
    void Shader::bind(unsigned int location, std::vector<glm::mat4> const & matrices) const
    { glUniformMatrix4fv(location, matrices.size(), GL_FALSE, glm::value_ptr(*matrices.data())); }

    void Shader::bind(unsigned int location, int value) const
    { glUniform1i(location, value); }
    void Shader::bind(unsigned int location, glm::vec3 const & value) const
    { glUniform3fv(location, 1, glm::value_ptr(value)); }
    void Shader::bind(unsigned int location, glm::vec4 const & value) const
    { glUniform4fv(location, 1, glm::value_ptr(value)); }

    Shader & Shader::attach(std::string const & filename)
    {
      // Load GLSL Shader Source from File
      std::string path = PROJECT_SOURCE_DIR "/Glitter/Shaders/";
      std::ifstream fd(path + filename);
      auto src = std::string(std::istreambuf_iterator<char>(fd),
                            (std::istreambuf_iterator<char>()));

      // Create a Shader Object
      const char * source = src.c_str();
      auto index = filename.rfind(".");
      auto ext = filename.substr(index + 1);
      GLenum type;
      if (ext == "comp") type = GL_COMPUTE_SHADER;
      else if (ext == "frag") type = GL_FRAGMENT_SHADER;
      else if (ext == "geom") type = GL_GEOMETRY_SHADER;
      else if (ext == "vert") type = GL_VERTEX_SHADER;
      else                    return *this;

      auto it = mProgramShaders.find(type);
      if (it != mProgramShaders.end())
      {
        GLuint shader = it->second;
        glDetachShader(mProgram, shader);
        mProgramShaders.erase(it);
      }

      GLuint shader = glCreateShader(type);
      glShaderSource(shader, 1, & source, nullptr);
      glCompileShader(shader);
      glGetShaderiv(shader, GL_COMPILE_STATUS, & mStatus);

      // Display the Build Log on Error
      if (mStatus == false)
      {
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, & mLength);
        std::unique_ptr<char[]> buffer(new char[mLength]);
        glGetShaderInfoLog(shader, mLength, nullptr, buffer.get());
        fprintf(stderr, "%s\n%s", filename.c_str(), buffer.get());
      }

      // Attach the Shader and Free Allocated Memory
      glAttachShader(mProgram, shader);
      glDeleteShader(shader);
      mProgramShaders[type] = shader;
      return *this;
    }

    Shader & Shader::link()
    {
        glLinkProgram(mProgram);
        glGetProgramiv(mProgram, GL_LINK_STATUS, & mStatus);
        if(mStatus == false)
        {
            glGetProgramiv(mProgram, GL_INFO_LOG_LENGTH, & mLength);
            std::unique_ptr<char[]> buffer(new char[mLength]);
            glGetProgramInfoLog(mProgram, mLength, nullptr, buffer.get());
            fprintf(stderr, "%s", buffer.get());
        }
        assert(mStatus == true);
        return *this;
    }
//};
