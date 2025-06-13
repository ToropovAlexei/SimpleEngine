#pragma once

#include <cstddef>
#include <glad/gl.h>

namespace engine::renderer {
class GLBuffer {
public:
  enum class Type : GLenum {
    Vertex = GL_ARRAY_BUFFER,
    Index = GL_ELEMENT_ARRAY_BUFFER,
    Uniform = GL_UNIFORM_BUFFER,
    ShaderStorage = GL_SHADER_STORAGE_BUFFER
  };

  enum class Usage : GLenum { Static = GL_STATIC_DRAW, Dynamic = GL_DYNAMIC_DRAW, Stream = GL_STREAM_DRAW };

public:
  GLBuffer(Type type, Usage usage, size_t size, const void *data = nullptr);
  ~GLBuffer();

  void bind();

  void bindVertexBuffer(size_t stride);

  unsigned int id() const { return m_id; }

private:
  unsigned int m_id;
  Type m_type;
  size_t m_size;
  Usage m_usage;
};
} // namespace engine::renderer