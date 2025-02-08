#pragma once

#include <GL/glew.h>

namespace pathtracer
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
