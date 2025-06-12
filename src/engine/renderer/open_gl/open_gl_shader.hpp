#pragma once
#include <glad/gl.h>
#include <vector>

namespace engine::renderer {
enum ShaderType {
  Vertex = GL_VERTEX_SHADER,
  Fragment = GL_FRAGMENT_SHADER,
};

class OpenGLShader {
public:
  OpenGLShader(const std::vector<char> &code, ShaderType type);
  ~OpenGLShader();

  unsigned int getId() { return m_shaderId; };

private:
  unsigned int m_shaderId;
};
} // namespace engine::renderer
