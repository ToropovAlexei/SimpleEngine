#include "gl_buffer.hpp"
#include <engine/core/assert.hpp>

namespace engine::renderer {
GLBuffer::GLBuffer(Type type, Usage usage, size_t size, const void *data)
    : m_type{type},
#ifndef NDEBUG
      m_size{size}, m_usage{usage}
#endif
{
  glCreateBuffers(1, &m_id);

  GLbitfield flags = 0;
  if (usage == Usage::Dynamic || usage == Usage::Stream) {
    flags |= GL_DYNAMIC_STORAGE_BIT;
  }

  glNamedBufferStorage(m_id, static_cast<GLsizeiptr>(size), data, flags);
}

GLBuffer::~GLBuffer() { glDeleteBuffers(1, &m_id); }

void GLBuffer::bind() { glBindBuffer(static_cast<GLenum>(m_type), m_id); }

void GLBuffer::bindBase(uint32_t index) { glBindBufferBase(static_cast<GLenum>(m_type), index, m_id); }

void GLBuffer::bindVertexBuffer(size_t stride) { glBindVertexBuffer(0, m_id, 0, static_cast<GLsizei>(stride)); }

void GLBuffer::update(size_t offset, size_t size, const void *data) {
#ifndef NDEBUG
  SE_ASSERT(m_size >= offset + size, "Buffer overflow");
  SE_ASSERT(m_usage == Usage::Dynamic || m_usage == Usage::Stream, "Buffer is not dynamic");
#endif
  glNamedBufferSubData(m_id, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), data);
}

} // namespace engine::renderer