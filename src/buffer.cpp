#include <pathtracer/buffer.hpp>

path_tracer::Buffer::Buffer(const GLenum target, const GLenum usage)
    : m_Target(target),
      m_Usage(usage)
{
    glGenBuffers(1, &m_Handle);
}

path_tracer::Buffer::~Buffer()
{
    glDeleteBuffers(1, &m_Handle);
}

void path_tracer::Buffer::Bind() const
{
    glBindBuffer(m_Target, m_Handle);
}

void path_tracer::Buffer::Unbind() const
{
    glBindBuffer(m_Target, 0);
}

void path_tracer::Buffer::Data(const GLsizeiptr size, const void *data) const
{
    glBufferData(m_Target, size, data, m_Usage);
}

void path_tracer::Buffer::BindBase(const GLuint i) const
{
    glBindBufferBase(m_Target, i, m_Handle);
}
