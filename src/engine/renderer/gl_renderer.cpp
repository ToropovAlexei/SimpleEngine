#include "gl_renderer.hpp"
#include <glad/gl.h>

namespace engine::renderer {
GlRenderer::GlRenderer(SDL_Window *window) : m_window{window} {
  m_glCtx = SDL_GL_CreateContext(m_window);
  gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress);
}

GlRenderer::~GlRenderer() { SDL_GL_DestroyContext(m_glCtx); }

void GlRenderer::beginFrame() {
  glClearColor(0.7f, 0.9f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
}

void GlRenderer::endFrame() { SDL_GL_SwapWindow(m_window); }
} // namespace engine::renderer
