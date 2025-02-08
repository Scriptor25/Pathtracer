#pragma once

#include <filesystem>
#include <functional>
#include <GL/glew.h>

namespace path_tracer
{
    typedef std::function<void(GLint loc)> UniformConsumer;

    class Shader
    {
    public:
        explicit Shader(const std::filesystem::path &path);

        ~Shader();

        void Bind() const;

        void Unbind() const;

        void SetUniform(const std::string &name, const UniformConsumer &consumer) const;

    private:
        GLuint m_Handle = 0;
    };
}
