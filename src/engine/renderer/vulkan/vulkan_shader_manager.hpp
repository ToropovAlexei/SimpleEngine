#pragma once

#include <engine/renderer/vulkan/vulkan_device.hpp>
#include <string>

namespace engine {
namespace renderer {
struct BindInfoPushConstant {
  vk::ShaderStageFlagBits stageFlags;
  uint32_t offset;
  uint32_t size;
};
struct BindInfo {
  std::string name;

  vk::DescriptorType descriptorType;
  uint32_t set;
  uint32_t binding;
  uint32_t count;
  vk::ShaderStageFlagBits stageFlags;

  bool isUsed = false;
  bool isWrite = false;
};
struct BindReflection {
  std::vector<vk::VertexInputBindingDescription2EXT> bindingDescriptions;
  std::vector<vk::VertexInputAttributeDescription2EXT> attributeDescriptions;
  std::vector<BindInfoPushConstant> pushConstants;
  std::vector<BindInfo> bindInfos;
};
struct Shader {
  std::string path;
  vk::ShaderModule shader;
  BindReflection bindReflection;
  std::vector<char> spirv;
};

class VulkanShaderManager {
public:
  enum class ShaderType { Vertex, Fragment };

public:
  VulkanShaderManager(VulkanDevice *device);
  ~VulkanShaderManager();

  size_t loadShader(std::string_view path, ShaderType type);

  VkShaderModule getVertexShaderModule(size_t index) { return m_vertexShaders[index].shader; }
  VkShaderModule getFragmentShaderModule(size_t index) { return m_fragmentShaders[index].shader; }

  const BindReflection &getVertexBindReflection(size_t index) { return m_vertexShaders[index].bindReflection; }
  const BindReflection &getFragmentBindReflection(size_t index) { return m_fragmentShaders[index].bindReflection; }

  std::vector<char> &getVertexSpirv(size_t index) { return m_vertexShaders[index].spirv; }
  std::vector<char> &getFragmentSpirv(size_t index) { return m_fragmentShaders[index].spirv; }

private:
  static std::vector<char> readFile(std::string_view filename);
  static BindReflection reflectBind(std::vector<char> &code);

  std::vector<Shader> &getShaders(ShaderType type);

private:
  VulkanDevice *m_device;
  std::vector<Shader> m_vertexShaders;
  std::vector<Shader> m_fragmentShaders;
};
} // namespace renderer
} // namespace engine
