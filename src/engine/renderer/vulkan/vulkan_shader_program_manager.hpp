#pragma once
#include <engine/renderer/vulkan/vulkan_shader_program.hpp>
#include <memory>

namespace engine::renderer {
class VulkanDevice;
class VulkanShaderProgramManager {
public:
  VulkanShaderProgramManager(VulkanDevice *device);

  ShaderProgramId createShaderProgram(VulkanShaderProgramDesc const &desc);

  void bindShaderProgram(VkCommandBuffer commandBuffer, ShaderProgramId shaderProgramId);

  VulkanShaderProgram *getShaderProgram(ShaderProgramId id) { return m_shaderPrograms[id.value].get(); }

private:
  VulkanDevice *m_device;
  std::vector<std::unique_ptr<VulkanShaderProgram>> m_shaderPrograms;
};
} // namespace engine::renderer