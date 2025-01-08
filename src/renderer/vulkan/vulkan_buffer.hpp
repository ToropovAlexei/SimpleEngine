#pragma once

#include <renderer/vulkan/vulkan_device.hpp>

class VulkanBuffer {
public:
  VulkanBuffer(VulkanDevice *device, VkDeviceSize instanceSize, uint32_t instanceCount,
               VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags flags = 0,
               VkDeviceSize minOffsetAlignment = 1);
  ~VulkanBuffer();

  VulkanBuffer(const VulkanBuffer &) = delete;
  VulkanBuffer &operator=(const VulkanBuffer &) = delete;

  void writeToBuffer(void *data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
  VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
  VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
  void invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

  void writeToIndex(void *data, VkDeviceSize index);
  VkResult flushIndex(VkDeviceSize index);
  VkDescriptorBufferInfo descriptorInfoForIndex(VkDeviceSize index);
  void invalidateIndex(VkDeviceSize index);

  inline VkBuffer getBuffer() const noexcept { return m_buffer; }
  inline uint32_t getInstanceCount() const noexcept { return m_instanceCount; }
  inline VkDeviceSize getInstanceSize() const noexcept { return m_instanceSize; }
  inline VkDeviceSize getAlignmentSize() const noexcept { return m_instanceSize; }
  inline VkBufferUsageFlags getUsageFlags() const noexcept { return m_usageFlags; }
  inline VmaMemoryUsage getMemoryPropertyFlags() const noexcept { return m_memoryUsage; }
  inline VkDeviceSize getBufferSize() const noexcept { return m_bufferSize; }

private:
  static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

  VulkanDevice *m_device;
  VkBuffer m_buffer = VK_NULL_HANDLE;
  VmaAllocation m_allocation = VK_NULL_HANDLE;
  VmaAllocationInfo m_allocationInfo{};

  VkDeviceSize m_bufferSize;
  uint32_t m_instanceCount;
  VkDeviceSize m_instanceSize;
  VkDeviceSize m_alignmentSize;
  VkBufferUsageFlags m_usageFlags;
  VmaMemoryUsage m_memoryUsage;
};
