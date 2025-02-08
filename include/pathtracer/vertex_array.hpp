#pragma once

#include <GL/glew.h>

namespace path_tracer
{
    class VertexArray
    {
    public:
        VertexArray();

        ~VertexArray();

        void Bind() const;

        void Unbind() const;

    private:
        GLuint m_Handle = 0;
    };
}
