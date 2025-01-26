#pragma once

#include <SDL3/SDL_video.h>
#include <optional>
#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace engine {
namespace renderer {
struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  bool graphicsFamilySupportsTimeStamps;

  std::optional<uint32_t> transferFamily;
  bool transferFamilySupportsTimeStamps;

  std::optional<uint32_t> presentFamily;
  bool presentFamilySupportsTimeStamps;

  std::optional<uint32_t> computeFamily;
  bool computeFamilySupportsTimeStamps;

  bool IsComplete() {
    return graphicsFamily.has_value() && transferFamily.has_value() && presentFamily.has_value() &&
           computeFamily.has_value();
  }
};

class VulkanDevice {
  friend class VulkanBufferManager;

public:
  VulkanDevice(SDL_Window *window);
  ~VulkanDevice();

  QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice device);
  inline QueueFamilyIndices findQueueFamilies() { return findQueueFamilies(m_physicalDevice); }
  SwapChainSupportDetails getSwapChainSupport();

  inline void flushGPU() { vkDeviceWaitIdle(m_device); }
  inline VkDevice &getDevice() noexcept { return m_device; };
  inline VkPhysicalDevice &getPhysicalDevice() noexcept { return m_physicalDevice; };
  inline VkSurfaceKHR &getSurface() noexcept { return m_surface; };
  inline VkQueue &getGraphicsQueue() noexcept { return m_graphicsQueue; };
  inline VkQueue &getTransferQueue() noexcept { return m_transferQueue; };
  inline VkQueue &getPresentQueue() noexcept { return m_presentQueue; };
  inline VmaAllocator &getAllocator() noexcept { return m_allocator; };
  inline VkCommandPool &getCommandPool() noexcept { return m_graphicsCommandPool; };
  inline VkInstance &getInstance() noexcept { return m_instance; };

  void createImageWithInfo(const VkImageCreateInfo &imageInfo, VmaMemoryUsage memoryUsage, VkImage &image,
                           VmaAllocation &imageAllocation);
  VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                               VkFormatFeatureFlags features);
  VkCommandBuffer beginSingleTimeCommands();
  void endSingleTimeCommands(VkCommandBuffer commandBuffer);
  void copyBuffer(VkBuffer dstBuffer, VkDeviceSize dstOffset, VkBuffer srcBuffer, VkDeviceSize srcOffset,
                  VkDeviceSize size);

  void transitionImageLayout(VkCommandBuffer cmdbuffer, VkImage image, VkAccessFlags srcAccessMask,
                             VkAccessFlags dstAccessMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout,
                             VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                             VkImageSubresourceRange subresourceRange);

private:
  void initVulkan();
  void setupDebugMessenger();
  void createSurface();
  void pickPhysicalDevice();
  void createLogicalDevice();
  void createAllocator();
  void createCommandPool();

  void checkValidationLayerSupport();
  bool checkInstanceExtensionSupport(std::vector<const char *> &requiredExtensions);
#ifndef NDEBUG
  void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
#endif
  std::vector<const char *> getRequiredExtensions();
  int rateDeviceSuitability(const VkPhysicalDevice &device);
  bool checkDeviceExtensionSupport(const VkPhysicalDevice &device);

private:
  static const uint32_t VK_API_VERSION = VK_API_VERSION_1_3; // TODO Change to 1.4 when VMA supports it

private:
  SDL_Window *m_window;
  VkSurfaceKHR m_surface;
  VkInstance m_instance;
  VkPhysicalDevice m_physicalDevice;
  VkDevice m_device;
  VmaAllocator m_allocator;

  VkQueue m_graphicsQueue;
  VkQueue m_transferQueue;
  VkQueue m_presentQueue;

  VkCommandPool m_graphicsCommandPool = VK_NULL_HANDLE;
  VkCommandPool m_transferCommandPool = VK_NULL_HANDLE;

#ifndef NDEBUG
  VkDebugUtilsMessengerEXT m_debugMessenger;
#endif
};
} // namespace renderer
} // namespace engine
