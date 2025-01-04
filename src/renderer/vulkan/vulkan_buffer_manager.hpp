#pragma once

#include "renderer/vulkan/vulkan_device.hpp"
#include <cstddef>
#include <queue>
#include <string_view>
#include <vector>
#include <vulkan/vulkan_core.h>

typedef size_t BufferId;

enum BufferUsage {
  INDIRECT_ARGUMENT_BUFFER = (1 << 0),
  STORAGE_BUFFER = (1 << 1),
  VERTEX_BUFFER = (1 << 2),
  INDEX_BUFFER = (1 << 3),
  UNIFORM_BUFFER = (1 << 4),
  TRANSFER_SOURCE = (1 << 5),
  TRANSFER_DESTINATION = (1 << 6),
};

enum class BufferCPUAccess {
  AccessNone,
  WriteOnly,
  ReadOnly,
};

struct Buffer {
  std::string name; // For debugging
  VmaAllocation allocation;
  VkBuffer buffer;
  VkDeviceSize size;
};

struct BufferDesc {
  std::string name = ""; // For debugging
  BufferUsage usage;
  BufferCPUAccess cpuAccess = BufferCPUAccess::AccessNone;
  size_t size = 0;
};

class VulkanBufferManager {
public:
  VulkanBufferManager(VulkanDevice *device);

  VkBuffer getBuffer(BufferId bufferId) const { return m_buffers[bufferId].buffer; }
  VkDeviceSize getBufferSize(BufferId bufferId) const { return m_buffers[bufferId].size; }
  VmaAllocation getBufferAllocation(BufferId bufferId) const { return m_buffers[bufferId].allocation; }
  std::string_view getBufferName(BufferId bufferId) const { return m_buffers[bufferId].name; }

  BufferId createBuffer(BufferDesc &desc);
  void destroyBuffer(BufferId bufferId);

  size_t getBufferCount() const { return m_buffers.size(); }

private:
  BufferId getNewBufferId();

private:
  VulkanDevice *m_device;

  std::vector<Buffer> m_buffers;
  std::queue<BufferId> m_freeIds;
};