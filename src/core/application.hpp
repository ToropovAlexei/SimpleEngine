#pragma once

#include <core/clock.hpp>
#include <core/input/keyboard.hpp>
#include <core/input/mouse.hpp>
#include <core/renderer.hpp>
#include <core/window.hpp>
#include <renderer/renderer_backend.hpp>
#include <string>
#include <string_view>

class Application {
public:
  Application(int width, int height, std::string_view name);
  ~Application();

  int run();

private:
  void handleEvents();
  void update(double dt);
  void render(double dt);

private:
  int m_width;
  int m_height;
  std::string m_name;
  bool m_running = true;
  bool m_isSuspended = false;
  Keyboard m_keyboard;
  Mouse m_mouse;
  Window m_window;
  RendererBackend m_renderer;
  Clock m_clock;
};