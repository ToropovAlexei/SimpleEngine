#pragma once

#include <engine/renderer/vulkan/vulkan_device.hpp>
#include <string>

namespace engine {
namespace renderer {
struct BindInfoPushConstant {
  uint32_t stageFlags;
  uint32_t offset;
  uint32_t size;
};
struct BindReflection {
  std::vector<VkVertexInputBindingDescription> bindingDescriptions;
  std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
  std::vector<BindInfoPushConstant> pushConstants;
};
struct Shader {
  std::string path;
  VkShaderModule shader;
  BindReflection bindReflection;
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
