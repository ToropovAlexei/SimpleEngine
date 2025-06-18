#include "gl_test_renderer.hpp"
#include "engine/core/assets_manager.hpp"
#include "engine/core/filesystem.hpp"
#include "engine/renderer/open_gl/gl_texture.hpp"
#include "engine/renderer/open_gl/open_gl_shader_program.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include <filesystem>
#include <functional>
#include <vector>

struct Vertex {
  float pos[3];
  float uv[2];
};

static std::vector<Vertex> vertices = { // Передняя грань (Z = 0.5)
    {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, 0.5f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f}},

    // Задняя грань (Z = -0.5)
    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
    {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f}},
    {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}},

    // Верхняя грань (Y = 0.5)
    {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f}},
    {{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}},
    {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}},
    {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f}},

    // Нижняя грань (Y = -0.5)
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
    {{0.5f, -0.5f, 0.5f}, {1.0f, 1.0f}},
    {{-0.5f, -0.5f, 0.5f}, {0.0f, 1.0f}},

    // Правая грань (X = 0.5)
    {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, 0.5f}, {1.0f, 1.0f}},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f}},

    // Левая грань (X = -0.5)
    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
    {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}},
    {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f}},
    {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}}};
static std::vector<unsigned int> indices = { // Передняя грань
    0, 1, 2, 0, 2, 3,
    // Задняя грань
    4, 5, 6, 4, 6, 7,
    // Верхняя грань
    8, 9, 10, 8, 10, 11,
    // Нижняя грань
    12, 13, 14, 12, 14, 15,
    // Правая грань
    16, 17, 18, 16, 18, 19,
    // Левая грань
    20, 21, 22, 20, 22, 23};

GlTestRenderer::GlTestRenderer(engine::renderer::GlRenderer *renderer) : m_renderer{renderer} {

  auto tex = engine::core::AssetsManager::loadTexture("wall.jpg");
  engine::renderer::GLTextureDesc desc = {};
  desc.type = engine::renderer::GLTextureType::Texture2D;
  desc.width = static_cast<uint32_t>(tex.width);
  desc.height = static_cast<uint32_t>(tex.height);
  desc.internalFormat = engine::renderer::GLTextureInternalFormat::RGB8;
  desc.format = tex.channels == 4 ? engine::renderer::GLTextureFormat::RGBA : engine::renderer::GLTextureFormat::RGB;
  desc.dataType = engine::renderer::GLTextureDataType::UByte;
  m_tex = std::make_unique<engine::renderer::GLTexture>(desc);
  m_tex->setData(tex.data.get());

  m_instances.resize(1000);
  for (size_t i = 0; i < m_instances.size(); ++i) {
    float x = (i % 30) * 1.5f - 15.0f;
    float z = (static_cast<float>(i) / 30) * 1.5f - 55.0f;
    m_instances[i].model = glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.0f, z));
  }

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
  m_ssbo = std::make_unique<engine::renderer::GLBuffer>(engine::renderer::GLBuffer::Type::ShaderStorage,
                                                        engine::renderer::GLBuffer::Usage::Dynamic,
                                                        m_instances.size() * sizeof(InstanceData), m_instances.data());
  m_uboData.model = glm::mat4(1.0f);
  m_uboData.model = glm::rotate(m_uboData.model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
  m_uboData.projection =
      glm::perspective(glm::radians(45.0f), static_cast<float>(m_width) / static_cast<float>(m_height), 0.1f, 1000.0f);
  m_uboData.view = glm::mat4(1.0f);
  m_uboData.view = glm::translate(m_uboData.view, glm::vec3(0.0f, 0.0f, -30.0f));
  m_ubo->update(0, sizeof(GlobalUBO), &m_uboData);

  m_vao->attachVertexBuffer(m_vbo.get(), 0, sizeof(Vertex), 0);
  m_vao->attachIndexBuffer(m_ibo.get());
  m_ubo->bindBase(0);
  m_ssbo->bindBase(2);
  m_tex->bind(1);

  m_vao->setAttributeFormat(0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));
  m_vao->bindAttribute(0, 0);
  m_vao->enableAttribute(0);

  m_vao->setAttributeFormat(1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv));
  m_vao->bindAttribute(1, 0);
  m_vao->enableAttribute(1);
  m_vao->bind();

#ifndef NDEBUG
  auto cbId = engine::core::AssetsManager::subscribe([this](std::string filename) { m_shouldReloadShaders = true; });
#endif

  reloadShaders();
}

void GlTestRenderer::reloadShaders() {
  auto shadersFolder = engine::core::getAbsolutePath(std::filesystem::path("assets/shaders"));
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
  glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr,
                          static_cast<GLsizei>(m_instances.size()));
}

void GlTestRenderer::resize(int width, int height) {
  m_width = width;
  m_height = height;
  m_uboData.projection =
      glm::perspective(glm::radians(60.0f), static_cast<float>(m_width) / static_cast<float>(m_height), 0.1f, 100.0f);
}

void GlTestRenderer::update(float dt) {
  m_uboData.elapsedTime += dt;
  m_uboData.model = glm::rotate(m_uboData.model, glm::radians(100.0f * dt), glm::vec3(1.0f, 1.0f, 1.0f));
  m_ubo->update(0, sizeof(GlobalUBO), &m_uboData);
}
