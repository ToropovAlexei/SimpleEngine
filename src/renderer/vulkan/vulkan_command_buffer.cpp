#include "vulkan_command_buffer.hpp"
#include <core/assert.hpp>
#include <vulkan/vulkan_core.h>

VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice device, VkCommandPool commandPool)
    : m_device{device}, m_commandPool{commandPool} {
  VkCommandBufferAllocateInfo allocInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .pNext = nullptr,
      .commandPool = m_commandPool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
  };
  vkAllocateCommandBuffers(m_device.getDevice(), &allocInfo, &m_commandBuffer);
}

VulkanCommandBuffer::~VulkanCommandBuffer() {
  if (m_commandBuffer) {
    vkFreeCommandBuffers(m_device.getDevice(), m_commandPool, 1, &m_commandBuffer);
  }
}

void VulkanCommandBuffer::begin(VkCommandBufferUsageFlags usage) {
  SE_ASSERT(!isRecording, "Command buffer is already recording.");

  VkCommandBufferBeginInfo beginInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .pNext = nullptr,
      .flags = usage,
      .pInheritanceInfo = nullptr,
  };

  vkBeginCommandBuffer(m_commandBuffer, &beginInfo);

#ifndef NDEBUG
  isRecording = true;
#endif
}

void VulkanCommandBuffer::end() {
  SE_ASSERT(isRecording, "Command buffer is not recording.");

  vkEndCommandBuffer(m_commandBuffer);

#ifndef NDEBUG
  isRecording = false;
#endif
}