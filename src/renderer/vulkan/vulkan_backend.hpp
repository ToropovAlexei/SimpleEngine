#pragma once

#include <memory>
#include <renderer/vulkan/vulkan_device.hpp>
#include <renderer/vulkan/vulkan_swapchain.hpp>

class VulkanBackend {
public:
  VulkanBackend(SDL_Window *window);
  ~VulkanBackend();

  void beginSwapChainRenderPass(vk::CommandBuffer commandBuffer);
  void endSwapChainRenderPass(vk::CommandBuffer commandBuffer);

  vk::CommandBuffer beginFrame();
  void endFrame();

  void onResize(int width, int height);

  inline float getAspectRatio() const { return m_swapChain->extentAspectRatio(); }
  inline VkRenderPass getSwapChainRenderPass() const { return m_swapChain->getRenderPass(); }
  inline size_t getFrameIndex() const {
    assert(m_isFrameStarted && "Cannog get frame index when frame not in progress");
    return m_currentFrameIndex;
  }
  inline vk::CommandBuffer getCurrentCommandBuffer() {
    assert(m_isFrameStarted && "Cannog get command buffer when frame not in progress");
    return m_commandBuffers[m_currentFrameIndex];
  }

private:
  void recreateSwapChain();
  void createCommandBuffers();

private:
  SDL_Window *m_window;
  std::unique_ptr<VulkanDevice> m_device;
  std::unique_ptr<VulkanSwapchain> m_swapChain;
  std::vector<vk::CommandBuffer> m_commandBuffers;

  uint32_t m_currentImageIndex = 0;
  size_t m_currentFrameIndex = 0;

#ifndef NDEBUG
  bool m_isFrameStarted = false;
#endif
};