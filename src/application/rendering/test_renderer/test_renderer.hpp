#pragma once

#include <engine/renderer/vulkan/vulkan_pipeline.hpp>
#include <engine/renderer/vulkan_renderer.hpp>
#include <glm/glm.hpp>
#include <memory>

class TestRenderer {
  struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;

    static std::vector<VkVertexInputBindingDescription> getBindingDescriptions() {
      return {{.binding = 0, .stride = sizeof(Vertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX}};
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
      return {{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos)},
              {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)}};
    }
  };

public:
  TestRenderer(engine::renderer::VulkanRenderer *device);
  ~TestRenderer();

  void render(VkCommandBuffer commandBuffer);

private:
  void createPipeline();

private:
  engine::renderer::VulkanRenderer *m_renderer;
  std::unique_ptr<engine::renderer::VulkanPipeline> m_pipeline;
};