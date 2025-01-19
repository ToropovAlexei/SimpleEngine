#include "vulkan_buffer_manager.hpp"
#include "vulkan_utils.hpp"

VulkanBufferManager::VulkanBufferManager(VulkanDevice *device) : m_device{device} {}

size_t VulkanBufferManager::createBuffer(BufferDesc &desc) {
  VkBufferUsageFlags usage = 0;

  if (desc.usage & BufferUsage::VERTEX_BUFFER) {
    usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  }

  if (desc.usage & BufferUsage::INDEX_BUFFER) {
    usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  }

  if (desc.usage & BufferUsage::UNIFORM_BUFFER) {
    usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  }

  if (desc.usage & BufferUsage::STORAGE_BUFFER) {
    usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  }

  if (desc.usage & BufferUsage::INDIRECT_ARGUMENT_BUFFER) {
    usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
  }

  if (desc.usage & BufferUsage::TRANSFER_SOURCE) {
    usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  }

  if (desc.usage & BufferUsage::TRANSFER_DESTINATION) {
    usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  }

  VkDeviceSize descSize = std::max(desc.size, 1ul);

  VkBufferCreateInfo bufferInfo = {};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = descSize;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
  if (desc.cpuAccess == BufferCPUAccess::ReadOnly) {
    memoryUsage = VMA_MEMORY_USAGE_GPU_TO_CPU;
  } else if (desc.cpuAccess == BufferCPUAccess::WriteOnly) {
    memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
  }

  VmaAllocationCreateInfo allocInfo = {};
  allocInfo.usage = memoryUsage;

  const size_t bufferId = getNewBufferId();

  Buffer &buffer = m_buffers[bufferId];
  buffer.size = descSize;

  VK_CHECK_RESULT(
      vmaCreateBuffer(m_device->m_allocator, &bufferInfo, &allocInfo, &buffer.buffer, &buffer.allocation, nullptr));

  buffer.name = desc.name;

  // TODO add debug marker

  return bufferId;
}

void VulkanBufferManager::destroyBuffer(size_t bufferId) {
  Buffer &buffer = m_buffers[bufferId];
  vmaDestroyBuffer(m_device->m_allocator, buffer.buffer, buffer.allocation);
  m_freeIds.push(bufferId);
}

size_t VulkanBufferManager::getNewBufferId() {
  if (m_freeIds.empty()) {
    size_t bufferId = m_buffers.size();
    m_buffers.emplace_back();
    return bufferId;
  }

  size_t bufferId = m_freeIds.front();
  m_freeIds.pop();
  return bufferId;
}