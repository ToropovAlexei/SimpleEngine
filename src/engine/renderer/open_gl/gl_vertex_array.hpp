#pragma once
#include "engine/renderer/open_gl/gl_buffer.hpp"
#include <glad/gl.h>

namespace engine::renderer {
class GLVertexArray {
public:
  GLVertexArray();
  ~GLVertexArray();

  void attachVertexBuffer(GLBuffer *buffer, uint32_t binding_index, size_t stride, size_t offset);

  void attachIndexBuffer(GLBuffer *buffer);

  void setAttributeFormat(uint32_t attribIndex, uint32_t size, GLenum type, GLboolean normalized,
                          uint32_t relativeOffset);

  void bindAttribute(uint32_t attribIndex, uint32_t bindingIndex);

  void enableAttribute(uint32_t attribIndex);

  void disableAttribute(uint32_t attribIndex);

  void bind() const;

private:
  unsigned int m_id;
};
} // namespace engine::renderer