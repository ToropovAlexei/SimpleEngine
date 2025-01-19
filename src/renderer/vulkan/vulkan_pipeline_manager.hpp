#pragma once

#include <renderer/vulkan/vulkan_device.hpp>

class VulkanPipelineManager {
public:
  VulkanPipelineManager();

private:
  VulkanDevice *m_device;
};