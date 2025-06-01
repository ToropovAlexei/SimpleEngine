#pragma once

#include <engine/renderer/descriptors/shader_program_descriptors.hpp>
#include <engine/renderer/vulkan/vulkan_device.hpp>
#include <vulkan/vulkan_enums.hpp>

namespace engine::renderer {
class VulkanShaderProgram {
public:
  VulkanShaderProgram(VulkanDevice *device, VulkanShaderProgramDesc const &desc);
  ~VulkanShaderProgram();

  void bind(vk::CommandBuffer commandBuffer);

  vk::PipelineLayout &getPipelineLayout() { return m_pipelineLayout; };

private:
  vk::ShaderCreateInfoEXT createShaderCreateInfo(const std::vector<char> &code,
                                                 VulkanShaderProgramDesc const &desc) const;

private:
  VulkanDevice *m_device;

  std::vector<vk::VertexInputAttributeDescription2EXT> m_attributes;
  std::vector<vk::VertexInputBindingDescription2EXT> m_bindings;
  std::vector<vk::ShaderStageFlagBits> m_stages;
  std::vector<vk::ShaderEXT> m_shaders;

  vk::PipelineLayout m_pipelineLayout;
};
} // namespace engine::renderer
