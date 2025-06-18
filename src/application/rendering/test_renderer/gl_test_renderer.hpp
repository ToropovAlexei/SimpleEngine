#pragma once

#include "engine/core/assert.hpp"
#include "engine/renderer/gl_renderer.hpp"
#include "engine/renderer/open_gl/gl_buffer.hpp"
#include "engine/renderer/open_gl/gl_texture.hpp"
#include "engine/renderer/open_gl/gl_vertex_array.hpp"
#include "engine/renderer/open_gl/open_gl_shader_program.hpp"
#include <efsw/efsw.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

struct GlobalUBO {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 projection;
  float elapsedTime;
};

struct InstanceData {
  glm::mat4 model;
};

class GlTestRenderer {
public:
  GlTestRenderer(engine::renderer::GlRenderer *renderer);

  void render();
  void update(float dt);

  void resize(int width, int height);
  void setView(const glm::mat4 &view) { m_uboData.view = view; }
  void setProjection(const glm::mat4 &projection) { m_uboData.projection = projection; }

private:
  void reloadShaders();

private:
  engine::renderer::GlRenderer *m_renderer;
  std::unique_ptr<engine::renderer::OpenGLShaderProgram> m_shader;
  std::unique_ptr<engine::renderer::GLBuffer> m_vbo;
  std::unique_ptr<engine::renderer::GLBuffer> m_ibo;
  std::unique_ptr<engine::renderer::GLVertexArray> m_vao;
  std::unique_ptr<engine::renderer::GLBuffer> m_ubo;
  std::unique_ptr<engine::renderer::GLTexture> m_tex;
  std::unique_ptr<engine::renderer::GLBuffer> m_ssbo;

  std::vector<InstanceData> m_instances;

  int m_width;
  int m_height;

  GlobalUBO m_uboData = {glm::mat4(1.0f), glm::mat4(1.0f), glm::mat4(1.0f), 0.0f};

  bool m_shouldReloadShaders = false;
};
