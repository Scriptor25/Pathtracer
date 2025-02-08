#pragma once

#include <GL/glew.h>

namespace path_tracer
{
    class Buffer
    {
    public:
        Buffer(GLenum target, GLenum usage);

        ~Buffer();

        void Bind() const;

        void Unbind() const;

        void Data(GLsizeiptr size, const void *data) const;

        void BindBase(GLuint i) const;

    private:
        GLuint m_Handle = 0;
        GLenum m_Target;
        GLenum m_Usage;
    };
}
