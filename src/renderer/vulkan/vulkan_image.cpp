#include "vulkan_image.hpp"
#include <core/exception.hpp>
#include <renderer/vulkan/vulkan_utils.hpp>

VulkanImage::VulkanImage(VulkanDevice device, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage)
    : m_device{device}, m_width{width}, m_height{height}, m_format{format}, m_usage{usage} {
  VkImageCreateInfo imageCreateInfo{
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = m_format,
      .extent = {m_width, m_height, 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = m_usage,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = nullptr,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };

  VmaAllocationCreateInfo allocCreateInfo = {};
  allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

  VmaAllocation allocation;
  VK_CHECK_RESULT(
      vmaCreateImage(m_device.getAllocator(), &imageCreateInfo, &allocCreateInfo, &m_image, &allocation, nullptr));

  m_allocationHandle = allocation;

  VkImageViewCreateInfo imageViewCreateInfo{
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .image = m_image,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = m_format,
      .components = {},
      .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
  };

  vkCreateImageView(m_device.getDevice(), &imageViewCreateInfo, nullptr, &m_imageView);

  VkSamplerCreateInfo samplerCreateInfo{
      .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .magFilter = VK_FILTER_LINEAR,
      .minFilter = VK_FILTER_LINEAR,
      .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
      .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
      .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
      .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
      .mipLodBias = 0.0f,
      .maxAnisotropy = 1.0f,
      .compareEnable = VK_FALSE,
      .compareOp = VK_COMPARE_OP_NEVER,
      .minLod = 0.0f,
      .maxLod = 0.0f,
      .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
      .unnormalizedCoordinates = VK_FALSE,
      .anisotropyEnable = VK_FALSE,
  };

  vkCreateSampler(m_device.getDevice(), &samplerCreateInfo, nullptr, &m_sampler);
}

VulkanImage::~VulkanImage() {
  if (m_imageView) {
    vkDestroyImageView(m_device.getDevice(), m_imageView, nullptr);
  }
  if (m_sampler) {
    vkDestroySampler(m_device.getDevice(), m_sampler, nullptr);
  }
  if (m_allocationHandle) {
    vmaDestroyImage(m_device.getAllocator(), m_image, m_allocationHandle);
  }
}