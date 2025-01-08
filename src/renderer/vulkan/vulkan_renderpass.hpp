#pragma once

#include <renderer/vulkan/vulkan_device.hpp>

class VulkanRenderPass {
public:
  VulkanRenderPass(VulkanDevice device, VkFormat swapchainFormat, VkFormat depthFormat);
  ~VulkanRenderPass();

  inline VkRenderPass getRenderPass() const noexcept { return m_renderPass; }

private:
  VulkanDevice m_device;
  VkRenderPass m_renderPass;
};