#pragma once

#include <core/exception.hpp>
#include <vulkan/vulkan.hpp>

#define VK_CHECK_RESULT(result)                                                                                        \
  do {                                                                                                                 \
    VkResult res = result;                                                                                             \
    if (res != VK_SUCCESS) {                                                                                           \
      SE_THROW_ERROR("Vulkan error: " + std::string(vk::to_string(vk::Result(res))));                                  \
    }                                                                                                                  \
  } while (false)
