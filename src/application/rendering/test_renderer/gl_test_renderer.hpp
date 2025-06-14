#pragma once

#include "engine/core/assert.hpp"
#include "engine/renderer/gl_renderer.hpp"
#include "engine/renderer/open_gl/gl_buffer.hpp"
#include "engine/renderer/open_gl/gl_texture.hpp"
#include "engine/renderer/open_gl/gl_vertex_array.hpp"
#include "engine/renderer/open_gl/open_gl_shader_program.hpp"
#include <efsw/efsw.hpp>
#include <fstream>
#include <functional>
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

class ShaderReloader : public efsw::FileWatchListener {
public:
  std::function<void()> onShaderModified;

  void handleFileAction(efsw::WatchID watchid, const std::string &dir, const std::string &filename, efsw::Action action,
                        std::string oldFilename = "") override {
    if (action == efsw::Actions::Modified) {
      std::cout << "Shader changed: " << filename << ", reloading..." << std::endl;
      onShaderModified();
    }
  }
};

class GlTestRenderer {
public:
  GlTestRenderer(engine::renderer::GlRenderer *renderer);

  void render();
  void update(float dt);

  std::vector<char> readFile(std::string_view filename) {
    std::ifstream file(filename.data(), std::ios::ate | std::ios::binary);

    SE_ASSERT(file.is_open(), "Failed to open shader file!");

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), static_cast<std::streamsize>(fileSize));

    file.close();
    return buffer;
  }

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
  std::unique_ptr<efsw::FileWatcher> m_fileWatcher;
  std::unique_ptr<ShaderReloader> m_shaderReloader;
  efsw::WatchID m_watchId = 0;
};
