#pragma once

#include <engine/renderer/vulkan/vulkan_device.hpp>
#include <string>

class VulkanShaderManager {
public:
  struct Shader {
    std::string path;
    VkShaderModule shader;
  };
  enum class ShaderType { Vertex, Fragment };

public:
  VulkanShaderManager(VulkanDevice *device);

  size_t loadShader(std::string_view path, ShaderType type);

private:
  static std::vector<char> readFile(std::string_view filename);

  std::vector<Shader> &getShaders(ShaderType type);

private:
  VulkanDevice *m_device;
  std::vector<Shader> m_vertexShaders;
  std::vector<Shader> m_fragmentShaders;
};