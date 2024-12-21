#pragma once

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
  Window m_window;
  Renderer m_renderer;
};