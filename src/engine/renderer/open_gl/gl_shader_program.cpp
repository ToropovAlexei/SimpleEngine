#include "gl_shader_program.hpp"
#include "engine/renderer/open_gl/gl_shader.hpp"

namespace engine::renderer {
GLShaderProgram::GLShaderProgram(const ShaderProgramCreateDesc desc) {
  m_shaderProgramId = glCreateProgram();
  auto vertexShader = GLShader(desc.vertexShaderCode, ShaderType::Vertex);
  auto fragmentShader = GLShader(desc.fragmentShaderCode, ShaderType::Fragment);
  glAttachShader(m_shaderProgramId, vertexShader.getId());
  glAttachShader(m_shaderProgramId, fragmentShader.getId());
  glLinkProgram(m_shaderProgramId);
}

GLShaderProgram::~GLShaderProgram() { glDeleteProgram(m_shaderProgramId); }

void GLShaderProgram::use() { glUseProgram(m_shaderProgramId); }
} // namespace engine::renderer
