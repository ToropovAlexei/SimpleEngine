#pragma once

#include <engine/renderer/vulkan/vulkan_device.hpp>
#include <string>

namespace engine {
namespace renderer {
class VulkanShaderManager {
public:
  struct Shader {
    std::string path;
    VkShaderModule shader;
  };
  enum class ShaderType { Vertex, Fragment };

public:
  VulkanShaderManager(VulkanDevice *device);
  ~VulkanShaderManager();

  size_t loadShader(std::string_view path, ShaderType type);

  VkShaderModule getVertexShaderModule(size_t index) { return m_vertexShaders[index].shader; }
  VkShaderModule getFragmentShaderModule(size_t index) { return m_fragmentShaders[index].shader; }

private:
  static std::vector<char> readFile(std::string_view filename);

  std::vector<Shader> &getShaders(ShaderType type);

private:
  VulkanDevice *m_device;
  std::vector<Shader> m_vertexShaders;
  std::vector<Shader> m_fragmentShaders;
};
} // namespace renderer
} // namespace engine
