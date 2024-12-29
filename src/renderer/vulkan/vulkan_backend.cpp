#include "vulkan_backend.hpp"
#include "SDL3/SDL_video.h"
#include <cstdint>

VulkanBackend::VulkanBackend(SDL_Window *window) : m_window{window} {
  m_device = std::make_unique<VulkanDevice>(m_window);
  int width = 0;
  int height = 0;
  SDL_GetWindowSize(m_window, &width, &height);
  m_swapChain = std::make_unique<VulkanSwapchain>(m_device.get(), vk::Extent2D{
                                                                      .width = static_cast<uint32_t>(width),
                                                                      .height = static_cast<uint32_t>(height),
                                                                  });
}

VulkanBackend::~VulkanBackend() {}