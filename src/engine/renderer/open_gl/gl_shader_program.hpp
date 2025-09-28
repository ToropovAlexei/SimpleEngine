#pragma once
#include <glad/gl.h>
#include <string>
#include <vector>

namespace engine::renderer {
struct ShaderProgramCreateDesc
{
  const std::vector<char> &fragmentShaderCode;
  const std::vector<char> &vertexShaderCode;
};

class GLShaderProgram
{
public:
  GLShaderProgram(const ShaderProgramCreateDesc desc);
  ~GLShaderProgram();

  void use() const;
  void setInt(const std::string &name, int value);

private:
  unsigned int m_shaderProgramId;
};
}// namespace engine::renderer
