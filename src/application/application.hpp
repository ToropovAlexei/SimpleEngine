#pragma once

#include <engine/core/input/keyboard.hpp>
#include <engine/core/input/mouse.hpp>
#include <engine/core/timer.hpp>
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
  void update(float dt);
  void render(float dt);

private:
  int m_width;
  int m_height;
  std::string m_name;
  bool m_running = true;
  bool m_isSuspended = false;
  engine::core::Keyboard m_keyboard;
  engine::core::Mouse m_mouse;
  engine::core::Window m_window;
  engine::core::Timer m_timer;
  engine::renderer::VulkanRenderer m_renderer;
};