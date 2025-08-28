#include "window.hpp"
#include <SDL3/SDL_video.h>
#include <engine/core/logger.hpp>

namespace engine::core {
Window::Window(int width, int height, std::string_view title)
{
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    Logger::fatal("Couldn't initialize SDL: {}", SDL_GetError());
    SDL_Quit();
  }
  Logger::info("SDL initialized");

  m_window = SDL_CreateWindow(title.data(), width, height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);

  if (m_window == nullptr) {
    Logger::fatal("Couldn't create window: {}", SDL_GetError());
    SDL_Quit();
  }
  Logger::info("Window created");
}

Window::~Window() { SDL_DestroyWindow(m_window); }

int Window::getWidth()
{
  int width;
  SDL_GetWindowSize(m_window, &width, nullptr);
  return width;
}

int Window::getHeight()
{
  int height;
  SDL_GetWindowSize(m_window, nullptr, &height);
  return height;
}
}// namespace engine::core
