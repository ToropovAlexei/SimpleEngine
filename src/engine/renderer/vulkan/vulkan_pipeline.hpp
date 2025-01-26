#pragma once

#include <engine/renderer/vulkan/vulkan_device.hpp>

namespace engine {
namespace renderer {
struct PipelineVkConfigInfo {
  std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
  std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
  std::vector<VkPushConstantRange> pushConstantRanges;
  VkPipelineViewportStateCreateInfo viewportInfo;
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
  VkPipelineRasterizationStateCreateInfo rasterizationInfo;
  VkPipelineMultisampleStateCreateInfo multisampleInfo;
  VkPipelineColorBlendAttachmentState colorBlendAttachment;
  VkPipelineColorBlendStateCreateInfo colorBlendInfo;
  VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
  std::vector<VkDynamicState> dynamicStateEnables;
  VkPipelineDynamicStateCreateInfo dynamicStateInfo;
  std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions;
  std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;
  VkPipelineLayout pipelineLayout = nullptr;
  VkRenderPass renderPass = nullptr;
  uint32_t subpass = 0;
};

class VulkanPipeline {
public:
  VulkanPipeline(VulkanDevice *device, const PipelineVkConfigInfo &configInfo);
  ~VulkanPipeline();

  static void defaultPipelineVkConfigInfo(PipelineVkConfigInfo &configInfo);

  void bind(VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);

private:
  void createPipelineVk(const PipelineVkConfigInfo &configInfo);

  VulkanDevice *m_device;
  VkPipeline m_graphicsPipeline;
  VkPipelineLayout m_pipelineLayout;
};
} // namespace renderer
} // namespace engine
