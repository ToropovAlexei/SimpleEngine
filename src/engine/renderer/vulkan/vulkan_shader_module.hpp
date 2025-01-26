#pragma once

#include "vulkan_device.hpp"
#include <string_view>

namespace engine {
namespace renderer {
class VulkanShaderModule {
public:
  VulkanShaderModule(VulkanDevice *device, std::string_view filePath, VkShaderStageFlagBits stage);
  ~VulkanShaderModule();

  VkPipelineShaderStageCreateInfo getShaderStageInfo() const;

private:
  static std::vector<char> readFile(std::string_view filename);
  void createShaderModule(const std::vector<char> &code);

private:
  VulkanDevice *m_device;
  VkShaderModule m_shaderModule{};
  VkShaderStageFlagBits m_stage;
};
} // namespace renderer
} // namespace engine
