#pragma once

#include "renderer/vulkan/vulkan_device.hpp"
#include <cstdint>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

class VulkanImage {
public:
  VulkanImage(VulkanDevice device, uint32_t width, uint32_t height, vk::Format format, vk::ImageUsageFlags usage);
  ~VulkanImage();

  inline vk::Image getImage() const noexcept { return m_image; }
  inline vk::ImageView getImageView() const noexcept { return m_imageView; }
  inline vk::Sampler getSampler() const noexcept { return m_sampler; }

private:
  VulkanDevice m_device;
  uint32_t m_width;
  uint32_t m_height;
  vk::Format m_format;
  vk::ImageUsageFlags m_usage;

  vk::Image m_image;
  vk::ImageView m_imageView;
  vk::Sampler m_sampler;

  VmaAllocation m_allocationHandle;
};