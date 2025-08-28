#pragma once

#include <cstddef>
#include <engine/core/assert.hpp>
#include <glad/gl.h>
#include <memory>

namespace engine::renderer {
class GLBuffer
{
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

  GLBuffer(const GLBuffer &) = delete;
  GLBuffer &operator=(const GLBuffer &) = delete;

  template<std::ranges::contiguous_range Range> static std::unique_ptr<GLBuffer> createVBO(const Range &range)
  {
    using T = std::ranges::range_value_t<Range>;
    return std::make_unique<GLBuffer>(Type::Vertex, Usage::Static, range.size() * sizeof(T), range.data());
  }

  template<std::ranges::contiguous_range Range> static std::unique_ptr<GLBuffer> createIBO(const Range &range)
  {
    using T = std::ranges::range_value_t<Range>;
    return std::make_unique<GLBuffer>(Type::Index, Usage::Static, range.size() * sizeof(T), range.data());
  }

  template<typename T> static std::unique_ptr<GLBuffer> createUBO(const T &data)
  {
    static_assert(alignof(T) == 16, "UBO must be aligned to 16 bytes");
    return std::make_unique<GLBuffer>(Type::Uniform, Usage::Dynamic, sizeof(T), &data);
  }

  template<std::ranges::contiguous_range Range> static std::unique_ptr<GLBuffer> createSSBO(const Range &range)
  {
    using T = std::ranges::range_value_t<Range>;
    return std::make_unique<GLBuffer>(Type::ShaderStorage, Usage::Dynamic, range.size() * sizeof(T), range.data());
  }

  void bind();

  void bindBase(uint32_t index);

  void bindVertexBuffer(size_t stride);

  template<typename T> void update(const T &data)
  {
    core::assertion(m_type == Type::Uniform, "This buffer is not a uniform buffer");
    static_assert(alignof(T) == 16, "UBO must be aligned to 16 bytes");
    update(0, sizeof(T), &data);
  }

  template<std::ranges::contiguous_range Range> void update(const Range &range)
  {
    core::assertion(m_type != Type::Uniform, "Cannot update uniform buffer this way");
    using T = std::ranges::range_value_t<Range>;
    update(0, range.size() * sizeof(T), range.data());
  }

  void update(size_t offset, size_t size, const void *data);

  unsigned int id() const { return m_id; }

private:
  unsigned int m_id;
  Type m_type;
#ifndef NDEBUG
  size_t m_size;
  Usage m_usage;
#endif
};
}// namespace engine::renderer