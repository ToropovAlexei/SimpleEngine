#pragma once

#include "engine/renderer/vulkan/vulkan_shader_manager.hpp"
#include <optional>
#include <vulkan/vulkan.hpp>

namespace engine::renderer {
struct ShaderProgramId {
  size_t value;
};
struct VulkanShaderProgramDesc {
  std::optional<std::vector<char>> vertexSpirv;
  std::optional<std::vector<char>> fragmentSpirv;
  std::vector<vk::VertexInputAttributeDescription2EXT> attributes;
  std::vector<vk::VertexInputBindingDescription2EXT> bindings;
  std::vector<vk::DescriptorSetLayout> setLayouts;
  std::vector<BindInfoPushConstant> pushConstants;
};

struct ShaderProgramDesc {
  size_t vertexShaderId;
  size_t fragmentShaderId;
};
} // namespace engine::renderer