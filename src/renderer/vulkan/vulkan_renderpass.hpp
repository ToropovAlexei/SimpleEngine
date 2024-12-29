#pragma once

#include <renderer/vulkan/vulkan_device.hpp>
#include <vulkan/vulkan.hpp>

class VulkanRenderPass {
public:
  VulkanRenderPass(VulkanDevice device, vk::Format swapchainFormat, vk::Format depthFormat);
  ~VulkanRenderPass();

  inline vk::RenderPass getRenderPass() const noexcept { return m_renderPass; }

private:
  VulkanDevice m_device;
  vk::RenderPass m_renderPass;
};