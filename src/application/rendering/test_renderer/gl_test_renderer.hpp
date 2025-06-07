#pragma once

#include "engine/renderer/gl_renderer.hpp"
#include <cstdint>

class GlTestRenderer {
public:
  GlTestRenderer(engine::renderer::GlRenderer *renderer);
  
  void render();
  void update(float dt);

private:
  engine::renderer::GlRenderer *m_renderer;
  std::uint32_t m_vbo;
  std::uint32_t m_vao;
};
