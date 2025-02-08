#include <pathtracer/vertex_array.hpp>

path_tracer::VertexArray::VertexArray()
{
    glGenVertexArrays(1, &m_Handle);
}

path_tracer::VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &m_Handle);
}

void path_tracer::VertexArray::Bind() const
{
    glBindVertexArray(m_Handle);
}

void path_tracer::VertexArray::Unbind() const
{
    glBindVertexArray(0);
}
