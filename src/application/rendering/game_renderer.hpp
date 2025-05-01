#pragma once
#include "application/rendering/test_renderer/test_renderer.hpp"
#include <engine/core/window.hpp>
#include <engine/renderer/vulkan/vulkan_descriptors.hpp>
#include <engine/renderer/vulkan_renderer.hpp>
#include <memory>

struct GlobalUBO {
  glm::mat4 projectionView;
};

class GameRenderer {
public:
  GameRenderer(engine::core::Window &window);

  void render(float dt);

  void updateRenderers(float dt);

  void setRenderSize(int width, int height);

private:
  engine::core::Window &m_window;
  std::unique_ptr<engine::renderer::VulkanRenderer> m_renderer;
  std::unique_ptr<engine::renderer::VulkanDescriptorPool> m_globalPool;
  GlobalUBO m_ubo;
  std::vector<VkDescriptorSet> m_globalDescriptorSets;
  std::vector<size_t> m_globalBufferIds;
  std::unique_ptr<TestRenderer> m_testRenderer;
};
