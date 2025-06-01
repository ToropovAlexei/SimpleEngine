#include "vulkan_shader_program_manager.hpp"

namespace engine::renderer {
VulkanShaderProgramManager::VulkanShaderProgramManager(VulkanDevice *device) : m_device{device} {}

ShaderProgramId VulkanShaderProgramManager::createShaderProgram(VulkanShaderProgramDesc const &desc) {
  m_shaderPrograms.emplace_back(std::make_unique<VulkanShaderProgram>(m_device, desc));
  return ShaderProgramId{m_shaderPrograms.size() - 1};
};

void VulkanShaderProgramManager::bindShaderProgram(VkCommandBuffer commandBuffer, ShaderProgramId shaderProgramId) {
  m_shaderPrograms[shaderProgramId.value]->bind(commandBuffer);
}
} // namespace engine::renderer