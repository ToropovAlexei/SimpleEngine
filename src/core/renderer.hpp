#pragma once

#include <SDL3/SDL_video.h>
#include <core/window.hpp>

class Renderer {
public:
  Renderer(SDL_Window *window);
  ~Renderer();

  void clear() { SDL_RenderClear(m_renderer); }
  void setDrawColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) { SDL_SetRenderDrawColor(m_renderer, r, g, b, a); }
  void present() { SDL_RenderPresent(m_renderer); }

private:
  SDL_Renderer *m_renderer;
};