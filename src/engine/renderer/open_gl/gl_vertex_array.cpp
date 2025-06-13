#include "gl_vertex_array.hpp"

namespace engine::renderer {
GLVertexArray::GLVertexArray() { glCreateVertexArrays(1, &m_id); }

GLVertexArray::~GLVertexArray() { glDeleteVertexArrays(1, &m_id); }

void GLVertexArray::attachVertexBuffer(GLBuffer *buffer, uint32_t binding_index, size_t stride, size_t offset) {
  glVertexArrayVertexBuffer(m_id, binding_index, buffer->id(), static_cast<GLintptr>(offset),
                            static_cast<GLsizei>(stride));
}

void GLVertexArray::attachIndexBuffer(GLBuffer *buffer) { glVertexArrayElementBuffer(m_id, buffer->id()); }

void GLVertexArray::bind() const { glBindVertexArray(m_id); }

void GLVertexArray::setAttributeFormat(uint32_t attribIndex, uint32_t size, GLenum type, GLboolean normalized,
                                       uint32_t relativeOffset) {
  glVertexArrayAttribFormat(m_id, attribIndex, static_cast<GLint>(size), type, normalized, relativeOffset);
}

void GLVertexArray::bindAttribute(uint32_t attribIndex, uint32_t bindingIndex) {
  glVertexArrayAttribBinding(m_id, attribIndex, bindingIndex);
}

void GLVertexArray::enableAttribute(uint32_t attribIndex) { glEnableVertexArrayAttrib(m_id, attribIndex); }

void GLVertexArray::disableAttribute(uint32_t attribIndex) { glDisableVertexArrayAttrib(m_id, attribIndex); }

} // namespace engine::renderer