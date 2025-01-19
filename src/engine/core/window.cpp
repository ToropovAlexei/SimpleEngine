#include "window.hpp"
#include <SDL3/SDL_video.h>
#include <engine/core/logger.hpp>

namespace engine {

namespace core {
Window::Window(int width, int height, std::string_view title) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    LOG_FATAL("Couldn't initialize SDL: {}", SDL_GetError());
    SDL_Quit();
  }
  LOG_INFO("SDL initialized");

  m_window = SDL_CreateWindow(title.data(), width, height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);

  if (!m_window) {
    LOG_FATAL("Couldn't create window: {}", SDL_GetError());
    SDL_Quit();
  }
  LOG_INFO("Window created");
}

Window::~Window() { SDL_DestroyWindow(m_window); }
} // namespace core
} // namespace engine
