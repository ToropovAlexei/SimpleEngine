#pragma once

#include "engine/core/assert.hpp"
#include "engine/renderer/gl_renderer.hpp"
#include "engine/renderer/open_gl/open_gl_shader_program.hpp"
#include <cstdint>
#include <efsw/efsw.hpp>
#include <fstream>
#include <functional>
#include <vector>

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

private:
  void reloadShaders();

private:
  engine::renderer::GlRenderer *m_renderer;
  std::unique_ptr<engine::renderer::OpenGLShaderProgram> m_shader;
  std::uint32_t m_vbo;
  std::uint32_t m_vao;

  bool m_shouldReloadShaders = false;
  std::unique_ptr<efsw::FileWatcher> m_fileWatcher;
  std::unique_ptr<ShaderReloader> m_shaderReloader;
  efsw::WatchID m_watchId = 0;
};
