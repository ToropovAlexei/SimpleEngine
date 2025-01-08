#pragma once

#include <renderer/vulkan/vulkan_device.hpp>
#include <vulkan/vulkan_core.h>

class VulkanCommandBuffer {
public:
  VulkanCommandBuffer(VulkanDevice device, VkCommandPool commandPool);
  ~VulkanCommandBuffer();

  void begin(VkCommandBufferUsageFlags usage = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
  void end();

  inline VkCommandBuffer getCommandBuffer() const noexcept { return m_commandBuffer; }

private:
  VulkanDevice m_device;
  VkCommandPool m_commandPool;
  VkCommandBuffer m_commandBuffer;

#ifndef NDEBUG
  bool isRecording = false;
#endif
};