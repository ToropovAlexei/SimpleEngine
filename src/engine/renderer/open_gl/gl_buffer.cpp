#include "gl_buffer.hpp"

namespace engine::renderer {
GLBuffer::GLBuffer(Type type, Usage usage, size_t size, const void *data) : m_type{type}, m_size{size}, m_usage{usage} {
  glCreateBuffers(1, &m_id);

  GLbitfield flags = 0;
  if (usage == Usage::Dynamic || usage == Usage::Stream) {
    flags |= GL_DYNAMIC_STORAGE_BIT;
  }

  glNamedBufferStorage(m_id, static_cast<GLsizeiptr>(size), data, flags);
}

GLBuffer::~GLBuffer() { glDeleteBuffers(1, &m_id); }

void GLBuffer::bind() { glBindBuffer(static_cast<GLenum>(m_type), m_id); }

void GLBuffer::bindVertexBuffer(size_t stride) { glBindVertexBuffer(0, m_id, 0, static_cast<GLsizei>(stride)); }

} // namespace engine::renderer