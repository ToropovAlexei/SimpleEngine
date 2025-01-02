#include "vulkan_shader_module.hpp"
#include "renderer/vulkan/vulkan_device.hpp"
#include <core/exception.hpp>
#include <fstream>
#include <string_view>

VulkanShaderModule::VulkanShaderModule(VulkanDevice *device, std::string_view filePath, vk::ShaderStageFlagBits stage)
    : m_device(device), m_stage(stage) {
  auto code = readFile(filePath);

  createShaderModule(code);
}

VulkanShaderModule::~VulkanShaderModule() {
  if (m_shaderModule) {
    m_device->getDevice().destroyShaderModule(m_shaderModule);
  }
}

vk::PipelineShaderStageCreateInfo VulkanShaderModule::getShaderStageInfo() const {
  return {
      .stage = m_stage,
      .module = m_shaderModule,
      .pName = "main",
  };
}

std::vector<char> VulkanShaderModule::readFile(std::string_view filename) {
  std::ifstream file(filename.data(), std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    SE_THROW_ERROR("Failed to open shader file!");
  }

  size_t fileSize = static_cast<size_t>(file.tellg());
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), static_cast<std::streamsize>(fileSize));

  file.close();
  return buffer;
}

void VulkanShaderModule::createShaderModule(const std::vector<char> &code) {
  vk::ShaderModuleCreateInfo createInfo = {
      .codeSize = code.size(),
      .pCode = reinterpret_cast<const uint32_t *>(code.data()),
  };

  m_shaderModule = m_device->getDevice().createShaderModule(createInfo);
  if (!m_shaderModule) {
    SE_THROW_ERROR("Failed to create shader module!");
  }
}