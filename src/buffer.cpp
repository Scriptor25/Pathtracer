#include <pathtracer/buffer.hpp>

pathtracer::Buffer::Buffer(const GLenum target, const GLenum usage)
    : m_Target(target), m_Usage(usage)
{
    glGenBuffers(1, &m_Handle);
}

pathtracer::Buffer::~Buffer()
{
    glDeleteBuffers(1, &m_Handle);
}

void pathtracer::Buffer::Bind() const
{
    glBindBuffer(m_Target, m_Handle);
}

void pathtracer::Buffer::Unbind() const
{
    glBindBuffer(m_Target, 0);
}

void pathtracer::Buffer::Data(const GLsizeiptr size, const void* pData) const
{
    glBufferData(m_Target, size, pData, m_Usage);
}

void pathtracer::Buffer::BindBase(const GLuint i) const
{
    glBindBufferBase(m_Target, i, m_Handle);
}
