#pragma once

// clang-format off
#include <glad/gl.h>
// clang-format on
#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>
#include <string_view>

namespace engine {

namespace core {
class Window {
public:
  Window(int width, int height, std::string_view title);
  ~Window();

  SDL_Window *getWindow() noexcept { return m_window; }

  int getWidth();
  int getHeight();

private:
  SDL_Window *m_window;
};
} // namespace core
} // namespace engine
