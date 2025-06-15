#pragma once

#include <glad/gl.h>

namespace engine::renderer {
enum class GLTextureType : GLenum {
  Texture2D = GL_TEXTURE_2D,
  Texture3D = GL_TEXTURE_3D,
  TextureCube = GL_TEXTURE_CUBE_MAP,
  Texture2DArray = GL_TEXTURE_2D_ARRAY
};
enum class GLTextureFormat : GLenum {
  R = GL_RED,
  RG = GL_RG,
  RGB = GL_RGB,
  RGBA = GL_RGBA,
  Depth = GL_DEPTH_COMPONENT,
  DepthStencil = GL_DEPTH_STENCIL
};

enum class GLTextureInternalFormat : GLenum {
  R8 = GL_R8,
  RGB8 = GL_RGB8,
  RGBA8 = GL_RGBA8,
  SRGB8 = GL_SRGB8,
  SRGB8_ALPHA8 = GL_SRGB8_ALPHA8,
  Depth16 = GL_DEPTH_COMPONENT16,
  Depth24 = GL_DEPTH_COMPONENT24,
  Depth32F = GL_DEPTH_COMPONENT32F
};

enum class GLTextureDataType : GLenum {
  UByte = GL_UNSIGNED_BYTE,
  Float = GL_FLOAT,
  UInt = GL_UNSIGNED_INT,
  UInt24_8 = GL_UNSIGNED_INT_24_8
};

enum class GLTextureFilter : GLenum {
  Nearest = GL_NEAREST,
  Linear = GL_LINEAR,
  MipmapLinear = GL_LINEAR_MIPMAP_LINEAR
};

enum class GlTextureWrapMode : GLenum {
  Repeat = GL_REPEAT,
  ClampToEdge = GL_CLAMP_TO_EDGE,
  MirroredRepeat = GL_MIRRORED_REPEAT
};

enum class GLTextureFace : GLenum {
  PosX = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
  NegX = GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
  PosY = GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
  NegY = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
  PosZ = GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
  NegZ = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};

struct GLTextureDesc {
  GLTextureType type;
  uint32_t width;
  uint32_t height;
  uint32_t depth;
  GLTextureInternalFormat internalFormat;
  GLTextureFormat format;
  GLTextureDataType dataType;
  GLTextureFilter minFilter = GLTextureFilter::Linear;
  GLTextureFilter magFilter = GLTextureFilter::Linear;
  GlTextureWrapMode wrapMode = GlTextureWrapMode::Repeat;
  bool generateMipmaps = false;
  bool anisotropicFiltering = true;
};

class GLTexture {
public:
public:
  GLTexture(const GLTextureDesc &desc);
  ~GLTexture();

  void bind(uint32_t unit) const;
  void setData(void *data, int level = 0);
  void setCubeFaceData(void *data, GLTextureFace face, int level = 0);

private:
  void setFilters(GLTextureFilter minFilter, GLTextureFilter magFilter);
  void generateMipmaps();
  void setWrapMode(GlTextureWrapMode wrapMode);
  void setAnisotropicFiltering();
  void allocateStorage();

private:
  unsigned int m_id;
  GLTextureType m_type;
  uint32_t m_width;
  uint32_t m_height;
  uint32_t m_depth;
  GLTextureInternalFormat m_internalFormat;
  GLTextureFormat m_format;
  GLTextureDataType m_dataType;
};
} // namespace engine::renderer