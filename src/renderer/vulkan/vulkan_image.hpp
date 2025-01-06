#pragma once

#include "renderer/vulkan/vulkan_device.hpp"
#include <cstdint>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

class VulkanImage {
public:
  VulkanImage(VulkanDevice device, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage);
  ~VulkanImage();

  inline VkImage getImage() const noexcept { return m_image; }
  inline VkImageView getImageView() const noexcept { return m_imageView; }
  inline VkSampler getSampler() const noexcept { return m_sampler; }

private:
  VulkanDevice m_device;
  uint32_t m_width;
  uint32_t m_height;
  VkFormat m_format;
  VkImageUsageFlags m_usage;

  VkImage m_image;
  VkImageView m_imageView;
  VkSampler m_sampler;

  VmaAllocation m_allocationHandle;
};