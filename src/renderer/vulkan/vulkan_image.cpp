#include "vulkan_image.hpp"
#include "renderer/vulkan/vulkan_utils.hpp"
#include <core/exception.hpp>
#include <vulkan/vulkan.hpp>

VulkanImage::VulkanImage(VulkanDevice device, uint32_t width, uint32_t height, vk::Format format,
                         vk::ImageUsageFlags usage)
    : m_device{device}, m_width{width}, m_height{height}, m_format{format}, m_usage{usage} {
  vk::ImageCreateInfo imageCreateInfo{.imageType = vk::ImageType::e2D,
                                      .format = m_format,
                                      .extent = {m_width, m_height, 1},
                                      .mipLevels = 1,
                                      .arrayLayers = 1,
                                      .samples = vk::SampleCountFlagBits::e1,
                                      .tiling = vk::ImageTiling::eOptimal,
                                      .usage = m_usage,
                                      .sharingMode = vk::SharingMode::eExclusive,
                                      .initialLayout = vk::ImageLayout::eUndefined};

  VmaAllocationCreateInfo allocCreateInfo = {};
  allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

  VkImage vkImage;
  VmaAllocation allocation;
  VK_CHECK_RESULT(vmaCreateImage(m_device.getAllocator(), reinterpret_cast<const VkImageCreateInfo *>(&imageCreateInfo),
                                 &allocCreateInfo, &vkImage, &allocation, nullptr));

  m_image = vk::Image(vkImage);
  m_allocationHandle = allocation;

  vk::ImageViewCreateInfo imageViewCreateInfo{
      .image = m_image,
      .viewType = vk::ImageViewType::e2D,
      .format = m_format,
      .subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1},
  };

  m_imageView = m_device.getDevice().createImageView(imageViewCreateInfo);

  vk::SamplerCreateInfo samplerCreateInfo{
      .magFilter = vk::Filter::eLinear,
      .minFilter = vk::Filter::eLinear,
      .addressModeU = vk::SamplerAddressMode::eClampToEdge,
      .addressModeV = vk::SamplerAddressMode::eClampToEdge,
      .addressModeW = vk::SamplerAddressMode::eClampToEdge,
      .mipLodBias = 0.0f,
      .maxAnisotropy = 1.0f,
      .compareEnable = vk::False,
      .compareOp = vk::CompareOp::eNever,
      .minLod = 0.0f,
      .maxLod = 0.0f,
      .borderColor = vk::BorderColor::eIntOpaqueBlack,
      .unnormalizedCoordinates = vk::False,
  };

  m_sampler = m_device.getDevice().createSampler(samplerCreateInfo);
}

VulkanImage::~VulkanImage() {
  if (m_imageView) {
    m_device.getDevice().destroyImageView(m_imageView);
  }
  if (m_sampler) {
    m_device.getDevice().destroySampler(m_sampler);
  }
  if (m_allocationHandle) {
    vmaDestroyImage(m_device.getAllocator(), m_image, m_allocationHandle);
  }
}