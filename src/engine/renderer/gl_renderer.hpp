#pragma once
#include <SDL3/SDL_video.h>

namespace engine::renderer {
class GlRenderer {
public:
  GlRenderer(SDL_Window *window);
  ~GlRenderer();

  void beginFrame();
  void endFrame();

  void onResize(int width, int height);

private:
  SDL_Window *m_window;
  SDL_GLContext m_glCtx;
};
} // namespace engine::renderer
