#pragma once

#include <engine/renderer/vulkan/vulkan_device.hpp>

namespace engine {
namespace renderer {
class VulkanPipelineManager {
public:
  VulkanPipelineManager();

private:
  VulkanDevice *m_device;
};
} // namespace renderer
} // namespace engine
