#include "vulkan_shader_manager.hpp"
#include "engine/core/filesystem.hpp"
#include "engine/core/logger.hpp"
#include "engine/renderer/vulkan/vulkan_utils.hpp"
#include <algorithm>
#include <engine/core/exception.hpp>
#include <fstream>

namespace engine {
namespace renderer {
VulkanShaderManager::VulkanShaderManager(VulkanDevice *device) : m_device(device) {}

VulkanShaderManager::~VulkanShaderManager() {
  for (auto &shader : m_vertexShaders) {
    vkDestroyShaderModule(m_device->getDevice(), shader.shader, nullptr);
  }
  for (auto &shader : m_fragmentShaders) {
    vkDestroyShaderModule(m_device->getDevice(), shader.shader, nullptr);
  }
}

std::vector<VulkanShaderManager::Shader> &VulkanShaderManager::getShaders(ShaderType type) {
  switch (type) {
  case ShaderType::Vertex:
    return m_vertexShaders;
  case ShaderType::Fragment:
    return m_fragmentShaders;
  }
}

size_t VulkanShaderManager::loadShader(std::string_view path, ShaderType type) {
  auto& shaders = getShaders(type);
  auto it = std::find_if(shaders.begin(), shaders.end(), [path](const Shader &shader) { return shader.path == path; });
  if (it != shaders.end()) {
    return static_cast<size_t>(it - shaders.begin());
  }

  const std::string filename = std::string(path) + (type == ShaderType::Vertex ? ".vert" : ".frag") + ".spv";
  const std::filesystem::path relativePath = std::filesystem::path("shaders") / filename;
  std::string absPathStr = core::getAbsolutePath(relativePath);

  auto code = readFile(absPathStr);

  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size();
  createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
  VkShaderModule shaderModule;

  VK_CHECK_RESULT(vkCreateShaderModule(m_device->getDevice(), &createInfo, nullptr, &shaderModule));

  shaders.push_back({std::string(path), shaderModule});
  return shaders.size() - 1;
}

std::vector<char> VulkanShaderManager::readFile(std::string_view filename) {
  std::ifstream file(filename.data(), std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    LOG_FATAL("Path not found: {}", filename);
    SE_THROW_ERROR("Failed to open shader file!");
  }

  size_t fileSize = static_cast<size_t>(file.tellg());
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), static_cast<std::streamsize>(fileSize));

  file.close();
  return buffer;
}
} // namespace renderer
} // namespace engine
