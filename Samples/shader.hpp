#pragma once

// System Headers
#include <glad/glad.h>
#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// Standard Headers
#include <string>
#include <vector>
#include <map>

// Define Namespace
//namespace Mirage
//{
    class Shader
    {
    public:

        // Implement Custom Constructor and Destructor
         Shader() { mProgram = glCreateProgram(); }
        ~Shader() { glDeleteProgram(mProgram); }

        // Public Member Functions
        Shader & activate();
        Shader & attach(std::string const & filename);
        GLuint   create(std::string const & filename);
        GLuint   get() const { return mProgram; }
        Shader & link();

        // Wrap Calls to glUniform
        void bind(unsigned int location, float value) const;
        void bind(unsigned int location, int value) const;
        void bind(unsigned int location, glm::vec3 const & vector) const;
        void bind(unsigned int location, glm::vec4 const & vector) const;
        void bind(unsigned int location, glm::mat4 const & matrix) const;
        void bind(unsigned int location, std::vector<glm::mat4> const & matrix) const;
        template<typename T> Shader const & bind(std::string const & name, T&& value) const
        {
            int location = glGetUniformLocation(mProgram, name.c_str());
            if (location == -1) fprintf(stderr, "Missing Uniform: \"%s\"\n", name.c_str());
            else bind(location, std::forward<T>(value));
            return *this;
        }

    private:

        // Disable Copying and Assignment
        Shader(Shader const &) = delete;
        Shader & operator=(Shader const &) = delete;

        // Private Member Variables
        GLuint mProgram;
        GLint  mStatus;
        GLint  mLength;
        std::map <GLuint, GLuint> mProgramShaders;

    };
//};
