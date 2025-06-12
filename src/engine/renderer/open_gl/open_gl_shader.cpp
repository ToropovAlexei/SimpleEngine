#include "open_gl_shader.hpp"
#include <string>

namespace engine::renderer {
OpenGLShader::OpenGLShader(const std::vector<char> &code, ShaderType type) {
  m_shaderId = glCreateShader(type);
  std::string shaderStr(code.data(), code.size());
  auto codeCStr = shaderStr.c_str();
  glShaderSource(m_shaderId, 1, &codeCStr, nullptr);
  glCompileShader(m_shaderId);
}

OpenGLShader::~OpenGLShader() {
  if (m_shaderId != 0) {
    glDeleteShader(m_shaderId);
  }
}
} // namespace engine::renderer
