#include "vulkan_buffer.hpp"

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
VkDeviceSize VulkanBuffer::getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) {
  if (minOffsetAlignment > 0) {
    return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
  }
  return instanceSize;
}

VulkanBuffer::VulkanBuffer(VulkanDevice *device, VkDeviceSize instanceSize, uint32_t instanceCount,
                           VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags flags,
                           VkDeviceSize minOffsetAlignment)
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
void VulkanBuffer::writeToBuffer(void *data, VkDeviceSize size, VkDeviceSize offset) {
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
VkResult VulkanBuffer::flush(VkDeviceSize size, VkDeviceSize offset) {
  if (m_memoryUsage == VMA_MEMORY_USAGE_GPU_ONLY) {
    return VK_SUCCESS; // GPU_ONLY memory doesn't need flush
  }

  vmaFlushAllocation(m_device->getAllocator(), m_allocation, offset, size);
  return VK_SUCCESS;
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
void VulkanBuffer::invalidate(VkDeviceSize size, VkDeviceSize offset) {
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
VkDescriptorBufferInfo VulkanBuffer::descriptorInfo(VkDeviceSize size, VkDeviceSize offset) {
  return VkDescriptorBufferInfo{
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
void VulkanBuffer::writeToIndex(void *data, VkDeviceSize index) {
  writeToBuffer(data, m_instanceSize, index * m_alignmentSize);
}

/**
 *  Flush the memory range at index * alignmentSize of the buffer to make it
 * visible to the device
 *
 * @param index Used in offset calculation
 *
 */
VkResult VulkanBuffer::flushIndex(VkDeviceSize index) { return flush(m_alignmentSize, index * m_alignmentSize); }

/**
 * Create a buffer info descriptor
 *
 * @param index Specifies the region given by index * alignmentSize
 *
 * @return VkDescriptorBufferInfo for instance at index
 */
VkDescriptorBufferInfo VulkanBuffer::descriptorInfoForIndex(VkDeviceSize index) {
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
void VulkanBuffer::invalidateIndex(VkDeviceSize index) { return invalidate(m_alignmentSize, index * m_alignmentSize); }
