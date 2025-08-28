#pragma once

#include <engine/renderer/vulkan/vulkan_device.hpp>

namespace engine::renderer {
class VulkanRenderPass
{
public:
  VulkanRenderPass(VulkanDevice device, VkFormat swapchainFormat, VkFormat depthFormat);
  ~VulkanRenderPass();

  VkRenderPass getRenderPass() const noexcept { return m_renderPass; }

private:
  VulkanDevice m_device;
  VkRenderPass m_renderPass;
};
}// namespace engine::renderer
