#include "gl_test_renderer.hpp"
#include "engine/core/filesystem.hpp"
#include "engine/renderer/open_gl/open_gl_shader_program.hpp"
#include <filesystem>
#include <functional>
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

  m_vbo = std::make_unique<engine::renderer::GLBuffer>(engine::renderer::GLBuffer::Type::Vertex,
                                                       engine::renderer::GLBuffer::Usage::Static,
                                                       vertices.size() * sizeof(Vertex), vertices.data());

  glGenVertexArrays(1, &m_vao);
  glBindVertexArray(m_vao);

  m_vbo->bindVertexBuffer(sizeof(Vertex));

  glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);
  glEnableVertexAttribArray(0);
  glVertexAttribBinding(0, 0);

  glVertexAttribFormat(1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, color));
  glEnableVertexAttribArray(1);
  glVertexAttribBinding(1, 0);

  auto shadersFolder = std::filesystem::path(SOURCE_DIR) / "shaders";
  m_shaderReloader = std::make_unique<ShaderReloader>();
  m_shaderReloader->onShaderModified = [this]() {
    auto sourceShadersPath = std::filesystem::path(SOURCE_DIR) / "shaders";
    auto targetShadersPath = engine::core::getAbsolutePath(std::filesystem::path("shaders"));

    for (const auto &entry : std::filesystem::directory_iterator(sourceShadersPath)) {
      std::filesystem::copy(entry.path(), targetShadersPath / entry.path().filename(),
                            std::filesystem::copy_options::overwrite_existing);
    }

    m_shouldReloadShaders = true;
  };

  m_fileWatcher = std::make_unique<efsw::FileWatcher>();
  m_watchId = m_fileWatcher->addWatch(shadersFolder.string(), m_shaderReloader.get(), true);
  m_fileWatcher->watch();

  reloadShaders();
}

void GlTestRenderer::reloadShaders() {
  auto shadersFolder = engine::core::getAbsolutePath(std::filesystem::path("shaders"));
  std::string vertexShaderPath = shadersFolder / "test.vert";
  auto vertexShaderCode = readFile(vertexShaderPath);
  std::string fragmentShaderPath = shadersFolder / "test.frag";
  auto fragmentShaderCode = readFile(fragmentShaderPath);
  engine::renderer::ShaderProgramCreateDesc desc = {fragmentShaderCode, vertexShaderCode};
  m_shader = std::make_unique<engine::renderer::OpenGLShaderProgram>(desc);
  m_shader->use();
}

void GlTestRenderer::render() {
  if (m_shouldReloadShaders) {
    reloadShaders();
    m_shouldReloadShaders = false;
  }
  glBindVertexArray(m_vao);
  glDrawArrays(GL_TRIANGLES, 0, 3);
}

void GlTestRenderer::update(float dt) {}
