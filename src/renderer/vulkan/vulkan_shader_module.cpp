#include "vulkan_shader_module.hpp"
#include "renderer/vulkan/vulkan_utils.hpp"
#include <core/exception.hpp>
#include <fstream>

VulkanShaderModule::VulkanShaderModule(VulkanDevice *device, std::string_view filePath, VkShaderStageFlagBits stage)
    : m_device(device), m_stage(stage) {
  auto code = readFile(filePath);

  createShaderModule(code);
}

VulkanShaderModule::~VulkanShaderModule() {
  if (m_shaderModule) {
    vkDestroyShaderModule(m_device->getDevice(), m_shaderModule, nullptr);
  }
}

VkPipelineShaderStageCreateInfo VulkanShaderModule::getShaderStageInfo() const {
  return {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .stage = m_stage,
      .module = m_shaderModule,
      .pName = "main",
      .pSpecializationInfo = nullptr,
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
  VkShaderModuleCreateInfo createInfo = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .codeSize = code.size(),
      .pCode = reinterpret_cast<const uint32_t *>(code.data()),
  };

  VK_CHECK_RESULT(vkCreateShaderModule(m_device->getDevice(), &createInfo, nullptr, &m_shaderModule));
}