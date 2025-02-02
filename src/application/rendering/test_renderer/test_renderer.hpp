#pragma once

#include <engine/renderer/vulkan/vulkan_pipeline.hpp>
#include <engine/renderer/vulkan_renderer.hpp>
#include <glm/glm.hpp>

class TestRenderer {
public:
  TestRenderer(engine::renderer::VulkanRenderer *device);
  ~TestRenderer();

  void render(VkCommandBuffer commandBuffer);

private:
  void createPipeline();

private:
  engine::renderer::VulkanRenderer *m_renderer;

  size_t m_pipelineId;

  size_t m_vertexBufferId;

  size_t m_vertexShaderId;
  size_t m_fragmentShaderId;
};