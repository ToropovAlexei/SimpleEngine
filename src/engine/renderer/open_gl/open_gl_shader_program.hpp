#pragma once
#include <glad/gl.h>
#include <vector>

namespace engine::renderer {
struct ShaderProgramCreateDesc {
  const std::vector<char> &fragmentShaderCode;
  const std::vector<char> &vertexShaderCode;
};

class OpenGLShaderProgram {
public:
  OpenGLShaderProgram(const ShaderProgramCreateDesc desc);
  ~OpenGLShaderProgram();
  
  void use();

private:
  unsigned int m_shaderProgramId;
};
} // namespace engine::renderer
