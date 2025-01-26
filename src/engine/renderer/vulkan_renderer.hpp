#pragma once

#include <engine/core/assert.hpp>
#include <engine/renderer/vulkan/vulkan_buffer_manager.hpp>
#include <engine/renderer/vulkan/vulkan_device.hpp>
#include <engine/renderer/vulkan/vulkan_swapchain.hpp>
#include <memory>

namespace engine {
namespace renderer {
class VulkanRenderer {
public:
  VulkanRenderer(SDL_Window *window);
  ~VulkanRenderer();

  void beginRendering(VkCommandBuffer commandBuffer);
  void endRendering(VkCommandBuffer commandBuffer);

  VkCommandBuffer beginFrame();
  void endFrame();

  void onResize(int width, int height);

  inline float getAspectRatio() const { return m_swapChain->extentAspectRatio(); }
  inline size_t getFrameIndex() const {
    SE_ASSERT(m_isFrameStarted, "Can't get frame index when frame not in progress");
    return m_currentFrameIndex;
  }
  inline VkCommandBuffer getCurrentCommandBuffer() {
    SE_ASSERT(m_isFrameStarted, "Can't get command buffer when frame not in progress");
    return m_commandBuffers[m_currentFrameIndex];
  }

private:
  void recreateSwapChain();
  void createCommandBuffers();

  void initImGui();

private:
  SDL_Window *m_window;
  std::unique_ptr<VulkanDevice> m_device;
  std::unique_ptr<VulkanSwapchain> m_swapChain;
  std::unique_ptr<VulkanBufferManager> m_bufferManager;
  std::vector<VkCommandBuffer> m_commandBuffers;

  uint32_t m_currentImageIndex = 0;
  size_t m_currentFrameIndex = 0;

#ifndef NDEBUG
  bool m_isFrameStarted = false;
#endif

  VkDescriptorPool m_imguiPool;
};
} // namespace renderer
} // namespace engine