#include "gl_texture.hpp"
#include <engine/core/assert.hpp>

namespace engine::renderer {
GLTexture::GLTexture(const GLTextureDesc &desc)
    : m_type{desc.type}, m_width{desc.width}, m_height{desc.height}, m_depth{desc.depth},
      m_internalFormat{desc.internalFormat}, m_format{desc.format}, m_dataType{desc.dataType} {
  glCreateTextures(static_cast<GLenum>(desc.type), 1, &m_id);

  setFilters(desc.minFilter, desc.magFilter);
  setWrapMode(desc.wrapMode);

  allocateStorage();

  if (desc.generateMipmaps) {
    generateMipmaps();
  }

  if (desc.anisotropicFiltering && desc.type == GLTextureType::Texture2D) {
    setAnisotropicFiltering();
  }
}

GLTexture::~GLTexture() { glDeleteTextures(1, &m_id); }

void GLTexture::bind(uint32_t unit) const { glBindTextureUnit(unit, m_id); }

void GLTexture::setData(void *data, int level) {
  switch (m_type) {
  case GLTextureType::Texture2D:
    glTextureSubImage2D(m_id, level, 0, 0, static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height),
                        static_cast<GLenum>(m_format), static_cast<GLenum>(m_dataType), data);
    break;
  case GLTextureType::Texture3D:
  case GLTextureType::Texture2DArray:
    glTextureSubImage3D(m_id, level, 0, 0, 0, static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height),
                        static_cast<GLsizei>(m_depth), static_cast<GLenum>(m_format), static_cast<GLenum>(m_dataType),
                        data);
    break;
  case GLTextureType::TextureCube:
    SE_UNREACHABLE("Use setCubeFaceData for cube maps");
  default:
    SE_UNREACHABLE("Unsupported texture type");
  }
}

void GLTexture::setCubeFaceData(void *data, GLTextureFace face, int level) {
  SE_ASSERT(m_type == GLTextureType::TextureCube, "setCubeFaceData only for cube maps");
  glTextureSubImage3D(m_id, level, 0, 0, static_cast<GLint>(static_cast<GLenum>(face) - GL_TEXTURE_CUBE_MAP_POSITIVE_X),
                      static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height), 1, static_cast<GLenum>(m_format),
                      static_cast<GLenum>(m_dataType), data);
}

void GLTexture::setFilters(GLTextureFilter minFilter, GLTextureFilter magFilter) {
  glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(minFilter));
  glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(magFilter));
}

void GLTexture::setWrapMode(GlTextureWrapMode wrapMode) {
  GLint wrap = static_cast<GLint>(wrapMode);
  glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, wrap);
  glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, wrap);
  if (m_type == GLTextureType::Texture3D || m_type == GLTextureType::Texture2DArray) {
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_R, wrap);
  }
}

void GLTexture::setAnisotropicFiltering() {
  GLfloat maxAnisotropy;
  glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAnisotropy);
  glTextureParameterf(m_id, GL_TEXTURE_MAX_ANISOTROPY, maxAnisotropy);
}

void GLTexture::allocateStorage() {
  switch (m_type) {
  case GLTextureType::Texture2D:
  case GLTextureType::TextureCube:
    glTextureStorage2D(m_id, 1, static_cast<GLenum>(m_internalFormat), static_cast<GLsizei>(m_width),
                       static_cast<GLsizei>(m_height));
    break;
  case GLTextureType::Texture3D:
  case GLTextureType::Texture2DArray:
    glTextureStorage3D(m_id, 1, static_cast<GLenum>(m_internalFormat), static_cast<GLsizei>(m_width),
                       static_cast<GLsizei>(m_height), static_cast<GLsizei>(m_depth));
    break;
  default:
    SE_UNREACHABLE("Unsupported texture type");
  }
}

void GLTexture::generateMipmaps() { glGenerateTextureMipmap(m_id); }
} // namespace engine::renderer