#include <pathtracer/vertex_array.hpp>

pathtracer::VertexArray::VertexArray()
{
    glGenVertexArrays(1, &m_Handle);
}

pathtracer::VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &m_Handle);
}

void pathtracer::VertexArray::Bind() const
{
    glBindVertexArray(m_Handle);
}

void pathtracer::VertexArray::Unbind() const
{
    glBindVertexArray(0);
}
