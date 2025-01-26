#pragma once
#include "application/rendering/test_renderer/test_renderer.hpp"
#include <engine/core/window.hpp>
#include <engine/renderer/vulkan_renderer.hpp>
#include <memory>

class GameRenderer {
public:
  GameRenderer(engine::core::Window &window);

  void render(float dt);

  void updateRenderers(float dt);

  void setRenderSize(int width, int height);

private:
  engine::core::Window &m_window;
  std::unique_ptr<engine::renderer::VulkanRenderer> m_renderer;
  std::unique_ptr<TestRenderer> m_testRenderer;
};