#include "vulkan_command_buffer.hpp"
#include <core/assert.hpp>

VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice device, vk::CommandPool commandPool)
    : m_device{device}, m_commandPool{commandPool} {
  vk::CommandBufferAllocateInfo allocInfo{
      .commandPool = m_commandPool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1};
  m_commandBuffer = m_device.getDevice().allocateCommandBuffers(allocInfo).front();
}

VulkanCommandBuffer::~VulkanCommandBuffer() {
  if (m_commandBuffer) {
    m_device.getDevice().freeCommandBuffers(m_commandPool, m_commandBuffer);
  }
}

void VulkanCommandBuffer::begin(vk::CommandBufferUsageFlags usage) {
  SE_ASSERT(!isRecording, "Command buffer is already recording.");

  vk::CommandBufferBeginInfo beginInfo;
  beginInfo.setFlags(usage);

  m_commandBuffer.begin(beginInfo);
#ifndef NDEBUG
  isRecording = true;
#endif
}

void VulkanCommandBuffer::end() {
  SE_ASSERT(isRecording, "Command buffer is not recording.");

  m_commandBuffer.end();
#ifndef NDEBUG
  isRecording = false;
#endif
}