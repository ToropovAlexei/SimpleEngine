#pragma once

#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <core/window.hpp>

class Renderer {
public:
  Renderer(SDL_Window *window);
  ~Renderer();

  inline void clear() { SDL_RenderClear(m_renderer); }
  inline void setDrawColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    SDL_SetRenderDrawColor(m_renderer, r, g, b, a);
  }
  inline void renderRect(SDL_FRect &&rect) { SDL_RenderFillRect(m_renderer, &rect); }
  inline void present() { SDL_RenderPresent(m_renderer); }
  inline SDL_Renderer *getRenderer() noexcept { return m_renderer; }

private:
  SDL_Renderer *m_renderer;
};