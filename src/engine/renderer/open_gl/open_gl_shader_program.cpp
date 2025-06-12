#include "open_gl_shader_program.hpp"
#include "engine/renderer/open_gl/open_gl_shader.hpp"

namespace engine::renderer {
OpenGLShaderProgram::OpenGLShaderProgram(const ShaderProgramCreateDesc desc) {
  m_shaderProgramId = glCreateProgram();
  auto vertexShader = OpenGLShader(desc.vertexShaderCode, ShaderType::Vertex);
  auto fragmentShader = OpenGLShader(desc.fragmentShaderCode, ShaderType::Fragment);
  glAttachShader(m_shaderProgramId, vertexShader.getId());
  glAttachShader(m_shaderProgramId, fragmentShader.getId());
  glLinkProgram(m_shaderProgramId);
}

OpenGLShaderProgram::~OpenGLShaderProgram() { glDeleteProgram(m_shaderProgramId); }

void OpenGLShaderProgram::use() { glUseProgram(m_shaderProgramId); }
} // namespace engine::renderer
