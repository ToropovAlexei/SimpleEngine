#pragma once

#include <core/input/keyboard.hpp>
#include <core/input/mouse.hpp>
#include <core/renderer.hpp>
#include <core/window.hpp>
#include <cstdint>
#include <string>
#include <string_view>

class Application {
public:
  Application(int width, int height, std::string_view name);
  ~Application();

  int run();

private:
  int m_width;
  int m_height;
  std::string m_name;
  bool m_running = true;
  bool m_isSuspended = false;
  uint64_t m_lastFrameTime;
  Keyboard m_keyboard;
  Mouse m_mouse;
  Window m_window;
  Renderer m_renderer;
};