#pragma once

#include <optional>
#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

struct SDL_Window;

namespace engine {
namespace renderer {
struct SwapChainSupportDetails {
  vk::SurfaceCapabilitiesKHR capabilities;
  std::vector<vk::SurfaceFormatKHR> formats;
  std::vector<vk::PresentModeKHR> presentModes;
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

  QueueFamilyIndices findQueueFamilies(const vk::PhysicalDevice device);
  inline QueueFamilyIndices findQueueFamilies() { return findQueueFamilies(m_physicalDevice); }
  SwapChainSupportDetails getSwapChainSupport();

  inline void flushGPU() { m_device.waitIdle(); }
  inline vk::Device &getDevice() noexcept { return m_device; };
  inline vk::PhysicalDevice &getPhysicalDevice() noexcept { return m_physicalDevice; };
  inline vk::SurfaceKHR &getSurface() noexcept { return m_surface; };
  inline vk::Queue &getGraphicsQueue() noexcept { return m_graphicsQueue; };
  inline vk::Queue &getTransferQueue() noexcept { return m_transferQueue; };
  inline vk::Queue &getPresentQueue() noexcept { return m_presentQueue; };
  inline VmaAllocator &getAllocator() noexcept { return m_allocator; };
  inline vk::CommandPool &getCommandPool() noexcept { return m_graphicsCommandPool; };
  inline vk::Instance &getInstance() noexcept { return m_instance; };

  void createImageWithInfo(const vk::ImageCreateInfo &imageInfo, VmaMemoryUsage memoryUsage, vk::Image &image,
                           VmaAllocation &imageAllocation);
  vk::Format findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling,
                                 vk::FormatFeatureFlags features);
  vk::CommandBuffer beginSingleTimeCommands();
  void endSingleTimeCommands(vk::CommandBuffer commandBuffer);
  void copyBuffer(vk::Buffer dstBuffer, vk::DeviceSize dstOffset, vk::Buffer srcBuffer, vk::DeviceSize srcOffset,
                  vk::DeviceSize size);

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
  int rateDeviceSuitability(const vk::PhysicalDevice &device);
  bool checkDeviceExtensionSupport(const vk::PhysicalDevice &device);

private:
  static constexpr auto VK_API_VERSION = VK_API_VERSION_1_4;

private:
  SDL_Window *m_window;
  vk::SurfaceKHR m_surface;
  vk::Instance m_instance;
  vk::PhysicalDevice m_physicalDevice;
  vk::Device m_device;
  VmaAllocator m_allocator;

  vk::Queue m_graphicsQueue;
  vk::Queue m_transferQueue;
  vk::Queue m_presentQueue;

  vk::CommandPool m_graphicsCommandPool = VK_NULL_HANDLE;
  vk::CommandPool m_transferCommandPool = VK_NULL_HANDLE;

#ifndef NDEBUG
  VkDebugUtilsMessengerEXT m_debugMessenger;
#endif
};
} // namespace renderer
} // namespace engine
