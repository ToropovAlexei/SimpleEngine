#pragma once

#include "engine/renderer/vulkan/vulkan_swapchain.hpp"
#include <engine/renderer/descriptors/pipeline_descriptors.hpp>
#include <engine/renderer/vulkan/vulkan_device.hpp>
#include <engine/renderer/vulkan/vulkan_shader_manager.hpp>

namespace engine {
namespace renderer {
class VulkanPipelineManager {
public:
  struct DescriptorSetLayoutData {
    VkDescriptorSetLayoutCreateInfo createInfo;
    std::vector<VkDescriptorSetLayoutBinding> bindings;
  };

  struct GraphicsPipeline {
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
    std::vector<VkPushConstantRange> pushConstantRanges;
  };

public:
  VulkanPipelineManager(VulkanDevice *device, VulkanShaderManager *shaderManager, VulkanSwapchain *swapchain);
  ~VulkanPipelineManager();

  size_t createGraphicsPipeline(GraphicsPipelineDesc &desc);

  VkPipeline getGraphicsPipeline(size_t index) { return m_graphicsPipelines[index].pipeline; }
  VkPipelineLayout getGraphicsPipelineLayout(size_t index) { return m_graphicsPipelines[index].pipelineLayout; }

private:
  VulkanDevice *m_device;
  VulkanShaderManager *m_shaderManager;
  VulkanSwapchain *m_swapchain;

  std::vector<GraphicsPipeline> m_graphicsPipelines;
};
} // namespace renderer
} // namespace engine
