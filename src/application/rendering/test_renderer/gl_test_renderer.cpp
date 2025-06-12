#include "gl_test_renderer.hpp"
#include "engine/core/filesystem.hpp"
#include "engine/renderer/open_gl/open_gl_shader_program.hpp"
#include <filesystem>
#include <vector>

struct Vertex {
  float pos[3];
  float color[3];
};

GlTestRenderer::GlTestRenderer(engine::renderer::GlRenderer *renderer) : m_renderer{renderer} {
  static std::vector<Vertex> vertices = {
      {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
      {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
      {{0.0f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
  };
  auto shadersFolder = engine::core::getAbsolutePath(std::filesystem::path("shaders"));
  std::string vertexShaderPath = shadersFolder / "test.vert";
  auto vertexShaderCode = readFile(vertexShaderPath);
  std::string fragmentShaderPath = shadersFolder / "test.frag";
  auto fragmentShaderCode = readFile(fragmentShaderPath);
  engine::renderer::ShaderProgramCreateDesc desc = {fragmentShaderCode, vertexShaderCode};
  m_shader = std::make_unique<engine::renderer::OpenGLShaderProgram>(desc);

  glGenVertexArrays(1, &m_vao);
  glGenBuffers(1, &m_vbo);
  glBindVertexArray(m_vao);

  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, static_cast<long>(vertices.size() * sizeof(Vertex)), vertices.data(), GL_STATIC_DRAW);
  glBindVertexBuffer(0, m_vbo, 0, sizeof(Vertex));

  glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);
  glEnableVertexAttribArray(0);
  glVertexAttribBinding(0, 0);

  glVertexAttribFormat(1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, color));
  glEnableVertexAttribArray(1);
  glVertexAttribBinding(1, 0);

  m_shader->use();
}

void GlTestRenderer::render() {
  glBindVertexArray(m_vao);
  glDrawArrays(GL_TRIANGLES, 0, 3);
}

void GlTestRenderer::update(float dt) {}
