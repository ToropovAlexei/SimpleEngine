#pragma once

#include <engine/renderer/vulkan/vulkan_device.hpp>

namespace engine {
namespace renderer {
class VulkanRenderPass {
public:
  VulkanRenderPass(VulkanDevice device, VkFormat swapchainFormat, VkFormat depthFormat);
  ~VulkanRenderPass();

  inline VkRenderPass getRenderPass() const noexcept { return m_renderPass; }

private:
  VulkanDevice m_device;
  VkRenderPass m_renderPass;
};
} // namespace renderer
} // namespace engine
