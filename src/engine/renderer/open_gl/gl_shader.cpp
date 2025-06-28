#include "gl_shader.hpp"
#include "engine/core/logger.hpp"
#include <string>

namespace engine::renderer {
GLShader::GLShader(const std::vector<char> &code, ShaderType type) {
  m_shaderId = glCreateShader(type);
  std::string shaderStr(code.data(), code.size());
  auto codeCStr = shaderStr.c_str();
  glShaderSource(m_shaderId, 1, &codeCStr, nullptr);
  glCompileShader(m_shaderId);

#ifndef NDEBUG
  GLint success = 0;
  glGetShaderiv(m_shaderId, GL_COMPILE_STATUS, &success);

  if (success == GL_FALSE) {
    GLint logLength = 0;
    glGetShaderiv(m_shaderId, GL_INFO_LOG_LENGTH, &logLength);

    std::vector<GLchar> errorLog(static_cast<size_t>(logLength));
    glGetShaderInfoLog(m_shaderId, logLength, nullptr, errorLog.data());

    LOG_ERROR("Shader compilation failed: {}", errorLog.data());
  }
#endif
}

GLShader::~GLShader() {
  if (m_shaderId != 0) {
    glDeleteShader(m_shaderId);
  }
}
} // namespace engine::renderer
