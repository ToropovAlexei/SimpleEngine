#include "vulkan_buffer.hpp"

#include <cassert>
#include <cstring>

/**
 * Returns the minimum instance size required to be compatible with devices
 * minOffsetAlignment
 *
 * @param instanceSize The size of an instance
 * @param minOffsetAlignment The minimum required alignment, in bytes, for the
 * offset member (eg minUniformBufferOffsetAlignment)
 *
 * @return VkResult of the buffer mapping call
 */
vk::DeviceSize VulkanBuffer::getAlignment(vk::DeviceSize instanceSize, vk::DeviceSize minOffsetAlignment) {
  if (minOffsetAlignment > 0) {
    return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
  }
  return instanceSize;
}

VulkanBuffer::VulkanBuffer(VulkanDevice *device, vk::DeviceSize instanceSize, uint32_t instanceCount,
                           vk::BufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags flags,
                           vk::DeviceSize minOffsetAlignment)
    : m_device{device}, m_instanceCount{instanceCount}, m_instanceSize{instanceSize}, m_usageFlags{usageFlags},
      m_memoryUsage{memoryUsage} {
  m_alignmentSize = getAlignment(instanceSize, minOffsetAlignment);
  m_bufferSize = m_alignmentSize * instanceCount;
  device->createBuffer(m_bufferSize, usageFlags, memoryUsage, flags, m_buffer, m_allocation, m_allocationInfo);
}

VulkanBuffer::~VulkanBuffer() {
  vmaDestroyBuffer(m_device->getAllocator(), static_cast<VkBuffer>(m_buffer), m_allocation);
}

/**
 * Copies the specified data to the mapped buffer. Default value writes whole
 * buffer range
 *
 * @param data Pointer to the data to copy
 * @param size (Optional) Size of the data to copy. Pass VK_WHOLE_SIZE to flush
 * the complete buffer range.
 * @param offset (Optional) Byte offset from beginning of mapped region
 *
 */
void VulkanBuffer::writeToBuffer(void *data, vk::DeviceSize size, vk::DeviceSize offset) {
  vmaCopyMemoryToAllocation(m_device->getAllocator(), data, m_allocation, offset,
                            size == VK_WHOLE_SIZE ? m_bufferSize : size);
}

/**
 * Flush a memory range of the buffer to make it visible to the device
 *
 * @note Only required for non-coherent memory
 *
 * @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE
 * to flush the complete buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the flush call
 */
vk::Result VulkanBuffer::flush(vk::DeviceSize size, vk::DeviceSize offset) {
  if (m_memoryUsage == VMA_MEMORY_USAGE_GPU_ONLY) {
    return vk::Result::eSuccess; // GPU_ONLY memory doesn't need flush
  }

  vmaFlushAllocation(m_device->getAllocator(), m_allocation, offset, size);
  return vk::Result::eSuccess;
}

/**
 * Invalidate a memory range of the buffer to make it visible to the host
 *
 * @note Only required for non-coherent memory
 *
 * @param size (Optional) Size of the memory range to invalidate. Pass
 * VK_WHOLE_SIZE to invalidate the complete buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the invalidate call
 */
void VulkanBuffer::invalidate(vk::DeviceSize size, vk::DeviceSize offset) {
  if (m_memoryUsage == VMA_MEMORY_USAGE_GPU_ONLY) {
    return; // GPU_ONLY memory doesn't need invalidation
  }

  vmaInvalidateAllocation(m_device->getAllocator(), m_allocation, offset, size);
}

/**
 * Create a buffer info descriptor
 *
 * @param size (Optional) Size of the memory range of the descriptor
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkDescriptorBufferInfo of specified offset and range
 */
vk::DescriptorBufferInfo VulkanBuffer::descriptorInfo(vk::DeviceSize size, vk::DeviceSize offset) {
  return vk::DescriptorBufferInfo{
      m_buffer,
      offset,
      size,
  };
}

/**
 * Copies "instanceSize" bytes of data to the mapped buffer at an offset of
 * index * alignmentSize
 *
 * @param data Pointer to the data to copy
 * @param index Used in offset calculation
 *
 */
void VulkanBuffer::writeToIndex(void *data, vk::DeviceSize index) {
  writeToBuffer(data, m_instanceSize, index * m_alignmentSize);
}

/**
 *  Flush the memory range at index * alignmentSize of the buffer to make it
 * visible to the device
 *
 * @param index Used in offset calculation
 *
 */
vk::Result VulkanBuffer::flushIndex(vk::DeviceSize index) { return flush(m_alignmentSize, index * m_alignmentSize); }

/**
 * Create a buffer info descriptor
 *
 * @param index Specifies the region given by index * alignmentSize
 *
 * @return VkDescriptorBufferInfo for instance at index
 */
vk::DescriptorBufferInfo VulkanBuffer::descriptorInfoForIndex(vk::DeviceSize index) {
  return descriptorInfo(m_alignmentSize, index * m_alignmentSize);
}

/**
 * Invalidate a memory range of the buffer to make it visible to the host
 *
 * @note Only required for non-coherent memory
 *
 * @param index Specifies the region to invalidate: index * alignmentSize
 *
 * @return VkResult of the invalidate call
 */
void VulkanBuffer::invalidateIndex(vk::DeviceSize index) {
  return invalidate(m_alignmentSize, index * m_alignmentSize);
}
