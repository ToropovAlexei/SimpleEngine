#include "window.hpp"
#include "core/logger.hpp"
#include <SDL3/SDL_video.h>

Window::Window(int width, int height, std::string_view title) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    LOG_FATAL("Couldn't initialize SDL: {}", SDL_GetError());
    SDL_Quit();
  }

  m_window = SDL_CreateWindow(title.data(), width, height, 0);

  if (!m_window) {
    LOG_FATAL("Couldn't create window: {}", SDL_GetError());
    SDL_Quit();
  }
}

Window::~Window() { SDL_DestroyWindow(m_window); }