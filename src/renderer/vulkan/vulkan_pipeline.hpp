#pragma once

#include "renderer/vulkan/vulkan_device.hpp"
#include <vector>

struct PipelineVkConfigInfo {
  std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
  std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
  std::vector<vk::PushConstantRange> pushConstantRanges;
  vk::PipelineViewportStateCreateInfo viewportInfo;
  vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
  vk::PipelineRasterizationStateCreateInfo rasterizationInfo;
  vk::PipelineMultisampleStateCreateInfo multisampleInfo;
  vk::PipelineColorBlendAttachmentState colorBlendAttachment;
  vk::PipelineColorBlendStateCreateInfo colorBlendInfo;
  vk::PipelineDepthStencilStateCreateInfo depthStencilInfo;
  std::vector<vk::DynamicState> dynamicStateEnables;
  vk::PipelineDynamicStateCreateInfo dynamicStateInfo;
  std::vector<vk::VertexInputBindingDescription> vertexInputBindingDescriptions;
  std::vector<vk::VertexInputAttributeDescription> vertexInputAttributeDescriptions;
  vk::PipelineLayout pipelineLayout = nullptr;
  vk::RenderPass renderPass = nullptr;
  uint32_t subpass = 0;
};

class VulkanPipeline {
public:
  VulkanPipeline(VulkanDevice *device, const PipelineVkConfigInfo &configInfo);
  ~VulkanPipeline();

  static void defaultPipelineVkConfigInfo(PipelineVkConfigInfo &configInfo);

  void bind(vk::CommandBuffer commandBuffer, vk::PipelineBindPoint bindPoint = vk::PipelineBindPoint::eGraphics);

private:
  void createPipelineVk(const PipelineVkConfigInfo &configInfo);

  VulkanDevice *m_device;
  vk::Pipeline m_graphicsPipeline;
  vk::PipelineLayout m_pipelineLayout;
};