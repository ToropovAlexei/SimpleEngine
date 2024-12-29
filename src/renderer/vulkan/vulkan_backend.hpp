#pragma once

#include "renderer/vulkan/vulkan_swapchain.hpp"
#include <memory>
#include <renderer/vulkan/vulkan_device.hpp>

class VulkanBackend {
public:
  VulkanBackend(SDL_Window *window);
  ~VulkanBackend();

private:
  SDL_Window *m_window;
  std::unique_ptr<VulkanDevice> m_device;
  std::unique_ptr<VulkanSwapchain> m_swapChain;
};