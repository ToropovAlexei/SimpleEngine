#pragma once

#include <engine/renderer/vulkan/vulkan_device.hpp>

namespace engine {
namespace renderer {
class VulkanCommandBuffer {
public:
  VulkanCommandBuffer(VulkanDevice device, vk::CommandPool commandPool);
  ~VulkanCommandBuffer();

  void begin(vk::CommandBufferUsageFlags usage = vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
  void end();

  inline vk::CommandBuffer getCommandBuffer() const noexcept { return m_commandBuffer; }

private:
  VulkanDevice m_device;
  vk::CommandPool m_commandPool;
  vk::CommandBuffer m_commandBuffer;

#ifndef NDEBUG
  bool isRecording = false;
#endif
};
} // namespace renderer
} // namespace engine
