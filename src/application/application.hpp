#pragma once

#include <engine/core/clock.hpp>
#include <engine/core/input/keyboard.hpp>
#include <engine/core/input/mouse.hpp>
#include <engine/core/window.hpp>
#include <engine/renderer/vulkan_renderer.hpp>
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
  engine::core::Keyboard m_keyboard;
  engine::core::Mouse m_mouse;
  engine::core::Window m_window;
  engine::renderer::VulkanRenderer m_renderer;
  engine::core::Clock m_clock;
};