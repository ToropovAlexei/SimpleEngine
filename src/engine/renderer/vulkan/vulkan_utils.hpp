#pragma once

#include <engine/core/exception.hpp>
#include <engine/renderer/descriptors/pipeline_descriptors.hpp>
#include <vulkan/vulkan.hpp>

#define VK_CHECK_RESULT(result)                                                                                        \
  do {                                                                                                                 \
    VkResult res = result;                                                                                             \
    if (res != VK_SUCCESS) {                                                                                           \
      SE_THROW_ERROR("Vulkan error: " + std::string(vk::to_string(vk::Result(res))));                                  \
    }                                                                                                                  \
  } while (false)

namespace engine {
namespace renderer {
class VulkanUtils {
public:
  VulkanUtils() = delete;

  static uint32_t getInputFormatSize(const InputFormat format) {
    switch (format) {
    // 4 bytes per component
    case InputFormat::R32G32B32A32_FLOAT:
      return 16;
    case InputFormat::R32G32B32A32_UINT:
      return 16;
    case InputFormat::R32G32B32A32_SINT:
      return 16;
    case InputFormat::R32G32B32_FLOAT:
      return 12;
    case InputFormat::R32G32B32_UINT:
      return 12;
    case InputFormat::R32G32B32_SINT:
      return 12;
    case InputFormat::R32G32_FLOAT:
      return 8;
    case InputFormat::R32G32_UINT:
      return 8;
    case InputFormat::R32G32_SINT:
      return 8;
    case InputFormat::R32_FLOAT:
      return 4;
    case InputFormat::R32_UINT:
      return 4;
    case InputFormat::R32_SINT:
      return 4;
    // 2 bytes per component
    case InputFormat::R16G16B16A16_FLOAT:
      return 8;
    case InputFormat::R16G16B16A16_UINT:
      return 8;
    case InputFormat::R16G16B16A16_SINT:
      return 8;
    case InputFormat::R16G16_FLOAT:
      return 4;
    case InputFormat::R16G16_UINT:
      return 4;
    case InputFormat::R16G16_SINT:
      return 4;
    case InputFormat::R16_FLOAT:
      return 2;
    case InputFormat::R16_UINT:
      return 2;
    case InputFormat::R16_SINT:
      return 2;
    // 1 byte per component
    case InputFormat::R8G8B8A8_UNORM:
      return 4;
    case InputFormat::R8G8B8A8_UINT:
      return 4;
    case InputFormat::R8G8B8A8_SINT:
      return 4;
    case InputFormat::R8G8_UINT:
      return 2;
    case InputFormat::R8G8_SINT:
      return 2;
    case InputFormat::R8_UINT:
      return 1;
    case InputFormat::R8_SINT:
      return 1;
    default:
      SE_THROW_ERROR("Invalid input format!");
    }

    return 1;
  }
};

} // namespace renderer
} // namespace engine
