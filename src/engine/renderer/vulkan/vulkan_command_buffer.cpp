#include "vulkan_command_buffer.hpp"
#include <engine/core/assert.hpp>

namespace engine {
namespace renderer {
VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice device, vk::CommandPool commandPool)
    : m_device{device}, m_commandPool{commandPool} {
  vk::CommandBufferAllocateInfo allocInfo = {
      .commandPool = m_commandPool,
      .level = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount = 1,
  };
  m_commandBuffer = m_device.getDevice().allocateCommandBuffers(allocInfo).value[0];
}

VulkanCommandBuffer::~VulkanCommandBuffer() {
  if (m_commandBuffer) {
    m_device.getDevice().freeCommandBuffers(m_commandPool, {m_commandBuffer});
  }
}

void VulkanCommandBuffer::begin(vk::CommandBufferUsageFlags usage) {
  SE_ASSERT(!isRecording, "Command buffer is already recording.");

  vk::CommandBufferBeginInfo beginInfo = {
      .flags = usage,
  };

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
} // namespace renderer
} // namespace engine
