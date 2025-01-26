#pragma once

#include <engine/renderer/descriptors/pipeline_descriptors.hpp>
#include <engine/renderer/vulkan/vulkan_device.hpp>
#include <engine/renderer/vulkan/vulkan_shader_manager.hpp>

namespace engine {
namespace renderer {
class VulkanPipelineManager {
public:
  struct GraphicsPipeline {
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
  };

public:
  VulkanPipelineManager(VulkanDevice *device, VulkanShaderManager *shaderManager);
  ~VulkanPipelineManager();

  size_t createGraphicsPipeline(GraphicsPipelineDesc &desc);

  VkPipeline getGraphicsPipeline(size_t index) { return m_graphicsPipelines[index].pipeline; }
  VkPipelineLayout getGraphicsPipelineLayout(size_t index) { return m_graphicsPipelines[index].pipelineLayout; }

private:
  VulkanDevice *m_device;
  VulkanShaderManager *m_shaderManager;

  std::vector<GraphicsPipeline> m_graphicsPipelines;
};
} // namespace renderer
} // namespace engine
