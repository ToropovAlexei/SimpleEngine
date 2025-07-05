#pragma once

#include "engine/core/assert.hpp"
#include "engine/renderer/gl_renderer.hpp"
#include "engine/renderer/open_gl/gl_buffer.hpp"
#include "engine/renderer/open_gl/gl_shader_program.hpp"
#include "engine/renderer/open_gl/gl_texture.hpp"
#include "engine/renderer/open_gl/gl_vertex_array.hpp"
#include <efsw/efsw.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

struct alignas(16) GlobalUBO {
  glm::mat4 view;
  glm::mat4 projection;
  alignas(16) glm::vec3 cameraPos;
  alignas(16) glm::vec3 lightPos;
  alignas(16) glm::vec3 lightColor;
  alignas(16) float elapsedTime;
};

struct alignas(16) Material {
  glm::vec3 ambient;
  float shininess;
  glm::vec3 diffuse;
  float opacity;
  glm::vec3 specular;
};

struct InstanceData {
  glm::mat4 model;
  alignas(16) uint32_t materialId;
};

class GlTestRenderer {
public:
  GlTestRenderer(engine::renderer::GlRenderer *renderer);

  void render();
  void update(float dt);

  void resize(int width, int height);
  void setView(const glm::mat4 &view) { m_uboData.view = view; }
  void setCameraPos(const glm::vec3 &pos) { m_uboData.cameraPos = pos; }
  void setProjection(const glm::mat4 &projection) { m_uboData.projection = projection; }

private:
  void reloadShaders();

private:
  [[maybe_unused]] engine::renderer::GlRenderer *m_renderer;
  std::unique_ptr<engine::renderer::GLShaderProgram> m_shader;
  std::unique_ptr<engine::renderer::GLShaderProgram> m_lightShader;
  std::unique_ptr<engine::renderer::GLBuffer> m_vbo;
  std::unique_ptr<engine::renderer::GLBuffer> m_ibo;
  std::unique_ptr<engine::renderer::GLVertexArray> m_vao;
  std::unique_ptr<engine::renderer::GLVertexArray> m_lightVAO;
  std::unique_ptr<engine::renderer::GLBuffer> m_ubo;
  std::unique_ptr<engine::renderer::GLTexture> m_tex;
  std::unique_ptr<engine::renderer::GLBuffer> m_ssbo;
  std::unique_ptr<engine::renderer::GLBuffer> m_materialsSSBO;

  std::vector<InstanceData> m_instances;
  std::vector<Material> m_materials;

  int m_width;
  int m_height;

  GlobalUBO m_uboData = {glm::mat4(1.0f),
                         glm::mat4(1.0f),
                         glm::vec3(0.0f, 0.0f, 0.0f),
                         glm::vec3(0.0f, 0.0f, 0.0f),
                         glm::vec3(1.0f, 1.0f, 1.0f),
                         0.0f};

  bool m_shouldReloadShaders = false;
};
