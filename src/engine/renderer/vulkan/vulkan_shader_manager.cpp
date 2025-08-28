#include "vulkan_shader_manager.hpp"
#include "engine/core/filesystem.hpp"
#include "engine/core/logger.hpp"
#include "engine/renderer/vulkan/vulkan_utils.hpp"
#include <algorithm>
#include <engine/core/assert.hpp>
#include <fstream>
#include <spirv_reflect.h>

namespace engine {
namespace renderer {
  VulkanShaderManager::VulkanShaderManager(VulkanDevice *device) : m_device(device) {}

  VulkanShaderManager::~VulkanShaderManager()
  {
    for (auto &shader : m_vertexShaders) { vkDestroyShaderModule(m_device->getDevice(), shader.shader, nullptr); }
    for (auto &shader : m_fragmentShaders) { vkDestroyShaderModule(m_device->getDevice(), shader.shader, nullptr); }
  }

  std::vector<Shader> &VulkanShaderManager::getShaders(ShaderType type)
  {
    switch (type) {
    case ShaderType::Vertex:
      return m_vertexShaders;
    case ShaderType::Fragment:
      return m_fragmentShaders;
    }
  }

  size_t VulkanShaderManager::loadShader(std::string_view path, ShaderType type)
  {
    auto &shaders = getShaders(type);
    auto it =
      std::find_if(shaders.begin(), shaders.end(), [path](const Shader &shader) { return shader.path == path; });
    if (it != shaders.end()) { return static_cast<size_t>(it - shaders.begin()); }

    const std::string filename = std::string(path) + (type == ShaderType::Vertex ? ".vert" : ".frag") + ".spv";
    const std::filesystem::path relativePath = std::filesystem::path("shaders") / filename;
    std::string absPathStr = core::getAbsolutePath(relativePath);

    auto code = readFile(absPathStr);

    vk::ShaderModuleCreateInfo createInfo{};
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    Shader shader;
    shader.path = path;
    shader.shader = m_device->getDevice().createShaderModule(createInfo).value;
    shader.spirv = code;

    shader.bindReflection = reflectBind(code);

    shaders.push_back(shader);
    return shaders.size() - 1;
  }

  std::vector<char> VulkanShaderManager::readFile(std::string_view filename)
  {
    std::ifstream file(filename.data(), std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
      core::Logger::fatal("Path not found: {}", filename);
      core::panic("Failed to open shader file!");
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), static_cast<std::streamsize>(fileSize));

    file.close();
    return buffer;
  }

  BindReflection VulkanShaderManager::reflectBind(std::vector<char> &code)
  {
    BindReflection reflection;
    SpvReflectShaderModule reflectModule;
    SpvReflectResult result = spvReflectCreateShaderModule(code.size(), code.data(), &reflectModule);

    core::assertion(result == SPV_REFLECT_RESULT_SUCCESS, "Spirv reflection failed!");

    uint32_t inputCount = 0;
    result = spvReflectEnumerateInputVariables(&reflectModule, &inputCount, NULL);

    core::assertion(result == SPV_REFLECT_RESULT_SUCCESS, "Spirv reflection failed!");

    if (inputCount > 0) {
      std::vector<SpvReflectInterfaceVariable *> inputVariables(inputCount);

      result = spvReflectEnumerateInputVariables(&reflectModule, &inputCount, inputVariables.data());

      core::assertion(result == SPV_REFLECT_RESULT_SUCCESS, "Spirv reflection failed!");

      vk::VertexInputBindingDescription2EXT bindingDescription = {};
      bindingDescription.binding = 0;
      bindingDescription.stride = 0;// computed below
      bindingDescription.inputRate = vk::VertexInputRate::eVertex;
      bindingDescription.divisor = 1;
      reflection.attributeDescriptions.reserve(inputVariables.size());
      for (const SpvReflectInterfaceVariable *var : inputVariables) {
        // ignore built-in variables
        if (var->decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN) { continue; }
        reflection.attributeDescriptions.push_back({
          .location = var->location,
          .binding = bindingDescription.binding,
          .format = static_cast<vk::Format>(var->format),
          .offset = 0,// final offset computed below after sorting.
        });
      }
      // Sort attributes by location
      std::sort(std::begin(reflection.attributeDescriptions),
        std::end(reflection.attributeDescriptions),
        [](const vk::VertexInputAttributeDescription2EXT &a, const vk::VertexInputAttributeDescription2EXT &b) {
          return a.location < b.location;
        });
      // Compute final offsets of each attribute, and total vertex stride.
      for (auto &attribute : reflection.attributeDescriptions) {
        uint32_t format_size = VulkanUtils::getInputFormatSize(static_cast<InputFormat>(attribute.format));
        attribute.offset = bindingDescription.stride;
        bindingDescription.stride += format_size;
      }
      reflection.bindingDescriptions.push_back(bindingDescription);
    }

    uint32_t outputCount = 0;
    result = spvReflectEnumerateOutputVariables(&reflectModule, &outputCount, NULL);

    if (result != SPV_REFLECT_RESULT_SUCCESS) { core::panic("Spirv reflection failed!"); }

    if (outputCount > 0) {
      std::vector<SpvReflectInterfaceVariable *> outputVariables(outputCount);

      result = spvReflectEnumerateOutputVariables(&reflectModule, &outputCount, outputVariables.data());

      if (result != SPV_REFLECT_RESULT_SUCCESS) { core::panic("Spirv reflection failed!"); }

      for (auto *outputVariable : outputVariables) {}
    }

    uint32_t pushConstantCount = 0;
    result = spvReflectEnumeratePushConstantBlocks(&reflectModule, &pushConstantCount, NULL);

    if (result != SPV_REFLECT_RESULT_SUCCESS) { core::panic("Spirv reflection failed!"); }

    if (pushConstantCount > 0) {
      std::vector<SpvReflectBlockVariable *> pushConstants(pushConstantCount);

      result = spvReflectEnumeratePushConstantBlocks(&reflectModule, &pushConstantCount, pushConstants.data());

      if (result != SPV_REFLECT_RESULT_SUCCESS) { core::panic("Spirv reflection failed!"); }

      for (SpvReflectBlockVariable *variable : pushConstants) {
        BindInfoPushConstant &pushConstant = reflection.pushConstants.emplace_back();
        pushConstant.offset = variable->offset;
        pushConstant.size = variable->size;
        pushConstant.stageFlags = static_cast<vk::ShaderStageFlagBits>(reflectModule.shader_stage);
      }
    }

    uint32_t descriptorSetCount = 0;
    result = spvReflectEnumerateDescriptorSets(&reflectModule, &descriptorSetCount, NULL);

    if (result != SPV_REFLECT_RESULT_SUCCESS) { core::panic("Spirv reflection failed!"); }

    if (descriptorSetCount > 0) {
      std::vector<SpvReflectDescriptorSet *> descriptorSets(descriptorSetCount);

      result = spvReflectEnumerateDescriptorSets(&reflectModule, &descriptorSetCount, descriptorSets.data());

      if (result != SPV_REFLECT_RESULT_SUCCESS) { core::panic("Spirv reflection failed!"); }

      for (auto *descriptorSet : descriptorSets) {
        for (uint32_t binding = 0; binding < descriptorSet->binding_count; binding++) {
          const SpvReflectDescriptorBinding *reflectionBinding = descriptorSet->bindings[binding];
          BindInfo bindInfo;
          bindInfo.descriptorType = static_cast<vk::DescriptorType>(reflectionBinding->descriptor_type);
          bindInfo.set = descriptorSet->set;
          bindInfo.binding = reflectionBinding->binding;
          bindInfo.count = reflectionBinding->count;
          bindInfo.stageFlags = static_cast<vk::ShaderStageFlagBits>(reflectModule.shader_stage);

          if (bindInfo.descriptorType == vk::DescriptorType::eStorageBuffer) {
            bindInfo.isUsed = reflectionBinding->accessed;
            bindInfo.isWrite = (reflectionBinding->resource_type & SPV_REFLECT_RESOURCE_FLAG_UAV);
          }

          bindInfo.name = reflectionBinding->name;

          reflection.bindInfos.push_back(bindInfo);
        }
      }
    }

    return reflection;
  }

}// namespace renderer
}// namespace engine
