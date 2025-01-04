#pragma once

#include <renderer/vulkan/vulkan_device.hpp>

class VulkanBuffer {
public:
  VulkanBuffer(VulkanDevice *device, vk::DeviceSize instanceSize, uint32_t instanceCount,
               vk::BufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags flags = 0,
               vk::DeviceSize minOffsetAlignment = 1);
  ~VulkanBuffer();

  VulkanBuffer(const VulkanBuffer &) = delete;
  VulkanBuffer &operator=(const VulkanBuffer &) = delete;

  void writeToBuffer(void *data, vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
  vk::Result flush(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
  vk::DescriptorBufferInfo descriptorInfo(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
  void invalidate(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);

  void writeToIndex(void *data, vk::DeviceSize index);
  vk::Result flushIndex(vk::DeviceSize index);
  vk::DescriptorBufferInfo descriptorInfoForIndex(vk::DeviceSize index);
  void invalidateIndex(vk::DeviceSize index);

  inline vk::Buffer getBuffer() const noexcept { return m_buffer; }
  inline uint32_t getInstanceCount() const noexcept { return m_instanceCount; }
  inline vk::DeviceSize getInstanceSize() const noexcept { return m_instanceSize; }
  inline vk::DeviceSize getAlignmentSize() const noexcept { return m_instanceSize; }
  inline vk::BufferUsageFlags getUsageFlags() const noexcept { return m_usageFlags; }
  inline VmaMemoryUsage getMemoryPropertyFlags() const noexcept { return m_memoryUsage; }
  inline vk::DeviceSize getBufferSize() const noexcept { return m_bufferSize; }

private:
  static vk::DeviceSize getAlignment(vk::DeviceSize instanceSize, vk::DeviceSize minOffsetAlignment);

  VulkanDevice *m_device;
  vk::Buffer m_buffer = VK_NULL_HANDLE;
  VmaAllocation m_allocation = VK_NULL_HANDLE;
  VmaAllocationInfo m_allocationInfo{};

  vk::DeviceSize m_bufferSize;
  uint32_t m_instanceCount;
  vk::DeviceSize m_instanceSize;
  vk::DeviceSize m_alignmentSize;
  vk::BufferUsageFlags m_usageFlags;
  VmaMemoryUsage m_memoryUsage;
};
