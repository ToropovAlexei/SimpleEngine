#pragma once
#include <engine/core/window.hpp>
#include <engine/renderer/vulkan_renderer.hpp>

class GameRenderer {
public:
  GameRenderer(engine::core::Window &window);

  void render(float dt);

  void updateRenderers(float dt);

  void setRenderSize(int width, int height);

private:
  engine::core::Window &m_window;
  engine::renderer::VulkanRenderer m_renderer;
};