#pragma once

#include "vulkan_device.hpp"
#include <vector>
#include <vulkan/vulkan.hpp>

class VulkanShaderModule {
public:
  VulkanShaderModule(VulkanDevice *device, std::string_view filePath, vk::ShaderStageFlagBits stage);
  ~VulkanShaderModule();

  vk::PipelineShaderStageCreateInfo getShaderStageInfo() const;

private:
  static std::vector<char> readFile(std::string_view filename);
  void createShaderModule(const std::vector<char> &code);

private:
  VulkanDevice *m_device;
  vk::ShaderModule m_shaderModule{};
  vk::ShaderStageFlagBits m_stage;
};