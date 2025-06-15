#include "gl_test_renderer.hpp"
#include "engine/core/filesystem.hpp"
#include "engine/renderer/open_gl/gl_texture.hpp"
#include "engine/renderer/open_gl/open_gl_shader_program.hpp"
#include "stb_image.h"
#include <filesystem>
#include <functional>
#include <vector>

struct Vertex {
  float pos[3];
  float uv[2];
};

GlTestRenderer::GlTestRenderer(engine::renderer::GlRenderer *renderer) : m_renderer{renderer} {
  static std::vector<Vertex> vertices = {
      {{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f}},
      {{0.5f, -0.5f, 0.0f}, {1.0f, 0.0f}},
      {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}},
      {{-0.5f, 0.5f, 0.0f}, {0.0f, 1.0f}},
  };
  static std::vector<unsigned int> indices = {0, 1, 3, 1, 2, 3};
  auto wallPath = engine::core::getAbsolutePath(std::filesystem::path("assets/wall.jpg"));
  int width;
  int height;
  int nrChannels;
  unsigned char *data = stbi_load(wallPath.c_str(), &width, &height, &nrChannels, 0);
  engine::renderer::GLTextureDesc desc = {};
  desc.type = engine::renderer::GLTextureType::Texture2D;
  desc.width = static_cast<uint32_t>(width);
  desc.height = static_cast<uint32_t>(height);
  desc.internalFormat = engine::renderer::GLTextureInternalFormat::RGB8;
  desc.format = engine::renderer::GLTextureFormat::RGB;
  desc.dataType = engine::renderer::GLTextureDataType::UByte;
  m_tex = std::make_unique<engine::renderer::GLTexture>(desc);
  m_tex->setData(data);
  stbi_image_free(data);

  m_vbo = std::make_unique<engine::renderer::GLBuffer>(engine::renderer::GLBuffer::Type::Vertex,
                                                       engine::renderer::GLBuffer::Usage::Static,
                                                       vertices.size() * sizeof(Vertex), vertices.data());
  m_ibo = std::make_unique<engine::renderer::GLBuffer>(engine::renderer::GLBuffer::Type::Index,
                                                       engine::renderer::GLBuffer::Usage::Static,
                                                       indices.size() * sizeof(unsigned int), indices.data());
  m_vao = std::make_unique<engine::renderer::GLVertexArray>();
  m_ubo = std::make_unique<engine::renderer::GLBuffer>(engine::renderer::GLBuffer::Type::Uniform,
                                                       engine::renderer::GLBuffer::Usage::Dynamic, sizeof(GlobalUBO),
                                                       nullptr);
  m_ubo->update(0, sizeof(GlobalUBO), &m_uboData);

  m_vao->attachVertexBuffer(m_vbo.get(), 0, sizeof(Vertex), 0);
  m_vao->attachIndexBuffer(m_ibo.get());
  m_ubo->bindBase(0);
  m_tex->bind(1);

  m_vao->setAttributeFormat(0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));
  m_vao->bindAttribute(0, 0);
  m_vao->enableAttribute(0);

  m_vao->setAttributeFormat(1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv));
  m_vao->bindAttribute(1, 0);
  m_vao->enableAttribute(1);
  m_vao->bind();

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
  m_vao->bind();
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

void GlTestRenderer::update(float dt) {
  m_uboData.elapsedTime += dt;
  m_ubo->update(0, sizeof(GlobalUBO), &m_uboData);
}
