#pragma once

#include <cstddef>
#include <engine/renderer/vulkan/vulkan_device.hpp>
#include <memory>

namespace engine {
namespace renderer {

class VulkanSwapchain {
public:
  static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

  VulkanSwapchain(VulkanDevice *device, VkExtent2D windowExtent);
  VulkanSwapchain(VulkanDevice *device, VkExtent2D windowExtent, std::shared_ptr<VulkanSwapchain> previousSwapChain);
  ~VulkanSwapchain();

  inline VkImageView getImageView(size_t index) noexcept { return m_swapChainImageViews[index]; }
  inline VkImage getImage(size_t index) noexcept { return m_swapChainImages[index]; }
  inline VkImageView getDepthImageView(size_t index) noexcept { return m_depthImageViews[index]; }
  inline VkImage getDepthImage(size_t index) noexcept { return m_depthImages[index]; }
  inline size_t imageCount() noexcept { return m_swapChainImages.size(); }
  inline VkFormat getSwapChainImageFormat() noexcept { return m_swapChainImageFormat; }
  inline VkExtent2D getSwapChainExtent() noexcept { return m_swapChainExtent; }
  inline uint32_t width() noexcept { return m_swapChainExtent.width; }
  inline uint32_t height() noexcept { return m_swapChainExtent.height; }

  inline float extentAspectRatio() noexcept { return m_aspectRatio; }
  VkFormat findDepthFormat();

  VkResult acquireNextImage(uint32_t *imageIndex);
  VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

  inline bool compareSwapFormats(const VulkanSwapchain &other) const noexcept {
    return other.m_swapChainDepthFormat == m_swapChainDepthFormat &&
           other.m_swapChainImageFormat == m_swapChainImageFormat;
  }

private:
  void init();
  void createSwapChain();
  void createImageViews();
  void createDepthResources();
  void createSyncObjects();

  VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
  VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

  VkFormat m_swapChainImageFormat;
  VkFormat m_swapChainDepthFormat;
  VkExtent2D m_swapChainExtent;
  float m_aspectRatio = 0.0f;

  std::vector<VkImage> m_depthImages;
  std::vector<VmaAllocation> m_depthImageMemorys;
  std::vector<VkImageView> m_depthImageViews;
  std::vector<VkImage> m_swapChainImages;
  std::vector<VkImageView> m_swapChainImageViews;

  VulkanDevice *m_device;
  VkExtent2D m_windowExtent;

  VkSwapchainKHR m_swapChain;
  std::shared_ptr<VulkanSwapchain> m_oldSwapChain;

  std::vector<VkSemaphore> m_imageAvailableSemaphores;
  std::vector<VkSemaphore> m_renderFinishedSemaphores;
  std::vector<VkFence> m_inFlightFences;
  std::vector<VkFence> m_imagesInFlight;
  size_t m_currentFrame = 0;
};
} // namespace renderer
} // namespace engine