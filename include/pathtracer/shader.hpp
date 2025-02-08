#pragma once

#include <filesystem>
#include <functional>
#include <GL/glew.h>

namespace pathtracer
{
    typedef std::function<void(GLint loc)> UniformCallback;

    class Shader
    {
    public:
        explicit Shader(const std::filesystem::path& path);
        ~Shader();

        void Bind() const;
        void Unbind() const;

        void SetUniform(const std::string& name, const UniformCallback& callback) const;

    private:
        GLuint m_Handle = 0;
    };
}
