#pragma once

#include "engine/core/assert.hpp"
#include "engine/renderer/gl_renderer.hpp"
#include "engine/renderer/open_gl/open_gl_shader_program.hpp"
#include <cstdint>
#include <fstream>
#include <vector>

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
  engine::renderer::GlRenderer *m_renderer;
  std::unique_ptr<engine::renderer::OpenGLShaderProgram> m_shader;
  std::uint32_t m_vbo;
  std::uint32_t m_vao;
};
