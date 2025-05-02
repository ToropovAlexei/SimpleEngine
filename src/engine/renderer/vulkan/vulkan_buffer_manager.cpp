#include "vulkan_buffer_manager.hpp"
#include "vulkan_utils.hpp"

namespace engine {
namespace renderer {
VulkanBufferManager::VulkanBufferManager(VulkanDevice *device) : m_device{device} {}

size_t VulkanBufferManager::createBuffer(BufferDesc &desc) {
  vk::BufferUsageFlags usage;
  VmaAllocationCreateInfo allocInfo = {};
  allocInfo.usage = VMA_MEMORY_USAGE_AUTO;

  if (desc.usage & BufferUsage::VERTEX_BUFFER) {
    usage |= vk::BufferUsageFlagBits::eVertexBuffer;
  }

  if (desc.usage & BufferUsage::INDEX_BUFFER) {
    usage |= vk::BufferUsageFlagBits::eIndexBuffer;
  }

  if (desc.usage & BufferUsage::UNIFORM_BUFFER) {
    usage |= vk::BufferUsageFlagBits::eUniformBuffer;
  }

  if (desc.usage & BufferUsage::STORAGE_BUFFER) {
    usage |= vk::BufferUsageFlagBits::eStorageBuffer;
  }

  if (desc.usage & BufferUsage::INDIRECT_ARGUMENT_BUFFER) {
    usage |= vk::BufferUsageFlagBits::eIndirectBuffer;
  }

  if (desc.usage & BufferUsage::TRANSFER_SOURCE) {
    usage |= vk::BufferUsageFlagBits::eTransferSrc;
    allocInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
  }

  if (desc.usage & BufferUsage::TRANSFER_DESTINATION) {
    usage |= vk::BufferUsageFlagBits::eTransferDst;
  }

  VkDeviceSize descSize = std::max(desc.size, 1ul);

  vk::BufferCreateInfo bufferInfo = {};
  bufferInfo.size = descSize;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = vk::SharingMode::eExclusive;

  const size_t bufferId = getNewBufferId();

  Buffer &buffer = m_buffers[bufferId];
  buffer.size = descSize;

  VkBufferCreateInfo rawInfo = bufferInfo;

  VkBuffer rawBuffer;
  VK_CHECK_RESULT(
      vmaCreateBuffer(m_device->m_allocator, &rawInfo, &allocInfo, &rawBuffer, &buffer.allocation, nullptr));
  buffer.buffer = rawBuffer;

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

VulkanBufferManager::~VulkanBufferManager() {
  for (auto &buffer : m_buffers) {
    vmaDestroyBuffer(m_device->m_allocator, buffer.buffer, buffer.allocation);
  }
}
} // namespace renderer
} // namespace engine
