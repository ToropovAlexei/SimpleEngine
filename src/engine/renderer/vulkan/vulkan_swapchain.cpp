#include "vulkan_swapchain.hpp"
#include <engine/core/logger.hpp>

namespace engine {
namespace renderer {
VulkanSwapchain::VulkanSwapchain(VulkanDevice *device, vk::Extent2D extent) : m_device{device}, m_windowExtent{extent} {
  init();
  LOG_INFO("Vulkan swapchain created");
}

VulkanSwapchain::VulkanSwapchain(VulkanDevice *device, vk::Extent2D extent,
                                 std::shared_ptr<VulkanSwapchain> previousSwapChain)
    : m_device{device}, m_windowExtent{extent}, m_oldSwapChain{previousSwapChain} {
  init();
  m_oldSwapChain = nullptr;
  LOG_INFO("Vulkan swapchain created");
}

void VulkanSwapchain::init() {
  createSwapChain();
  createImageViews();
  createDepthResources();
  createSyncObjects();
}

VulkanSwapchain::~VulkanSwapchain() {
  for (auto imageView : m_swapChainImageViews) {
    m_device->getDevice().destroyImageView(imageView);
  }
  m_swapChainImageViews.clear();

  if (m_swapChain != nullptr) {
    m_device->getDevice().destroySwapchainKHR(m_swapChain);
    m_swapChain = nullptr;
  }

  for (size_t i = 0; i < m_depthImages.size(); i++) {
    m_device->getDevice().destroyImageView(m_depthImageViews[i]);
    vmaDestroyImage(m_device->getAllocator(), m_depthImages[i], m_depthImageMemorys[i]);
  }

  // cleanup synchronization objects
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    m_device->getDevice().destroySemaphore(m_renderFinishedSemaphores[i]);
    m_device->getDevice().destroySemaphore(m_imageAvailableSemaphores[i]);
    m_device->getDevice().destroyFence(m_inFlightFences[i]);
  }

  LOG_INFO("Vulkan swapchain destroyed");
}

vk::Result VulkanSwapchain::acquireNextImage(uint32_t *imageIndex) {
  m_device->getDevice().waitForFences({m_inFlightFences[m_currentFrame]}, vk::True,
                                      std::numeric_limits<uint64_t>::max());

  auto result = m_device->getDevice().acquireNextImageKHR(m_swapChain, std::numeric_limits<uint64_t>::max(),
                                                          m_imageAvailableSemaphores[m_currentFrame]);

  if (result.result == vk::Result::eSuccess) {
    *imageIndex = result.value;
  }

  return result.result;
}

vk::Result VulkanSwapchain::submitCommandBuffers(const vk::CommandBuffer *buffers, uint32_t *imageIndex) {
  if (m_imagesInFlight[*imageIndex] != nullptr) {
    m_device->getDevice().waitForFences(m_imagesInFlight[*imageIndex], vk::True, std::numeric_limits<uint64_t>::max());
  }
  m_imagesInFlight[*imageIndex] = m_inFlightFences[m_currentFrame];

  vk::SubmitInfo submitInfo = {};

  vk::Semaphore waitSemaphores[] = {m_imageAvailableSemaphores[m_currentFrame]};
  vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = buffers;

  auto signalSemaphores = std::to_array({m_renderFinishedSemaphores[m_currentFrame]});
  submitInfo.signalSemaphoreCount = signalSemaphores.size();
  submitInfo.pSignalSemaphores = signalSemaphores.data();

  m_device->getDevice().resetFences(m_inFlightFences[m_currentFrame]);

  m_device->getGraphicsQueue().submit(submitInfo, m_inFlightFences[m_currentFrame]);

  vk::PresentInfoKHR presentInfo = {};
  presentInfo.waitSemaphoreCount = signalSemaphores.size();
  presentInfo.pWaitSemaphores = signalSemaphores.data();

  auto swapChains = std::to_array({m_swapChain});
  presentInfo.swapchainCount = swapChains.size();
  presentInfo.pSwapchains = swapChains.data();

  presentInfo.pImageIndices = imageIndex;

  auto result = m_device->getPresentQueue().presentKHR(presentInfo);

  m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

  return result;
}

void VulkanSwapchain::createSwapChain() {
  SwapChainSupportDetails swapChainSupport = m_device->getSwapChainSupport();

  vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
  vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
  vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  vk::SwapchainCreateInfoKHR createInfo = {};
  createInfo.surface = m_device->getSurface();
  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

  QueueFamilyIndices indices = m_device->findQueueFamilies();
  uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

  if (indices.graphicsFamily != indices.presentFamily) {
    createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    createInfo.imageSharingMode = vk::SharingMode::eExclusive;
    createInfo.queueFamilyIndexCount = 0;     // Optional
    createInfo.pQueueFamilyIndices = nullptr; // Optional
  }

  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;

  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;

  createInfo.oldSwapchain = m_oldSwapChain ? m_oldSwapChain->m_swapChain : VK_NULL_HANDLE;

  m_swapChain = m_device->getDevice().createSwapchainKHR(createInfo).value;

  // we only specified a minimum number of images in the swap chain, so the
  // implementation is allowed to create a swap chain with more. That's why
  // we'll first query the final number of images with vkGetSwapchainImagesKHR,
  // then resize the container and finally call it again to retrieve the
  // handles.
  uint32_t swapChainImagesCount;
  m_swapChainImages = m_device->getDevice().getSwapchainImagesKHR(m_swapChain).value;

  m_swapChainImageFormat = surfaceFormat.format;
  m_swapChainExtent = extent;
  m_aspectRatio = static_cast<float>(m_swapChainExtent.width) / static_cast<float>(m_swapChainExtent.height);
}

void VulkanSwapchain::createImageViews() {
  m_swapChainImageViews.resize(m_swapChainImages.size());
  for (size_t i = 0; i < m_swapChainImages.size(); i++) {
    vk::ImageViewCreateInfo viewInfo{};
    viewInfo.viewType = vk::ImageViewType::e2D;
    viewInfo.image = m_swapChainImages[i];
    viewInfo.format = m_swapChainImageFormat;
    viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    m_swapChainImageViews[i] = m_device->getDevice().createImageView(viewInfo).value;
  }
}

void VulkanSwapchain::createDepthResources() {
  vk::Format depthFormat = findDepthFormat();
  m_swapChainDepthFormat = depthFormat;
  vk::Extent2D swapChainExtent = getSwapChainExtent();

  m_depthImages.resize(imageCount());
  m_depthImageMemorys.resize(imageCount());
  m_depthImageViews.resize(imageCount());

  for (size_t i = 0; i < m_depthImages.size(); i++) {
    vk::ImageCreateInfo imageInfo = {
        .imageType = vk::ImageType::e2D,
        .format = depthFormat,
        .extent = {.width = swapChainExtent.width, .height = swapChainExtent.height, .depth = 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = vk::SampleCountFlagBits::e1,
        .tiling = vk::ImageTiling::eOptimal,
        .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
        .sharingMode = vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .initialLayout = vk::ImageLayout::eUndefined,
    };

    m_device->createImageWithInfo(imageInfo, VMA_MEMORY_USAGE_AUTO, m_depthImages[i], m_depthImageMemorys[i]);

    vk::ImageViewCreateInfo viewInfo = {.image = m_depthImages[i],
                                        .viewType = vk::ImageViewType::e2D,
                                        .format = depthFormat,
                                        .components = {},
                                        .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eDepth,
                                                             .baseMipLevel = 0,
                                                             .levelCount = 1,
                                                             .baseArrayLayer = 0,
                                                             .layerCount = 1}};

    m_depthImageViews[i] = m_device->getDevice().createImageView(viewInfo).value;
  }
}

void VulkanSwapchain::createSyncObjects() {
  m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
  m_imagesInFlight.resize(imageCount(), VK_NULL_HANDLE);

  vk::SemaphoreCreateInfo semaphoreInfo = {};

  vk::FenceCreateInfo fenceInfo = {
      .flags = vk::FenceCreateFlagBits::eSignaled,
  };

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    m_imageAvailableSemaphores[i] = m_device->getDevice().createSemaphore(semaphoreInfo).value;
    m_renderFinishedSemaphores[i] = m_device->getDevice().createSemaphore(semaphoreInfo).value;
    m_inFlightFences[i] = m_device->getDevice().createFence(fenceInfo).value;
  }
}

vk::SurfaceFormatKHR
VulkanSwapchain::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats) {
  for (const auto &availableFormat : availableFormats) {
    if (availableFormat.format == vk::Format::eB8G8R8A8Unorm &&
        availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
      return availableFormat;
    }
  }

  return availableFormats[0];
}

vk::PresentModeKHR
VulkanSwapchain::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes) {
  for (const auto &availablePresentMode : availablePresentModes) {
    if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
      LOG_INFO("Present mode: Mailbox");
      return availablePresentMode;
    }
  }

  LOG_INFO("Present mode: V-Sync");
  return vk::PresentModeKHR::eFifo;
}

vk::Extent2D VulkanSwapchain::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities) {
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    vk::Extent2D actualExtent = m_windowExtent;
    actualExtent.width =
        std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height =
        std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return actualExtent;
  }
}

vk::Format VulkanSwapchain::findDepthFormat() {
  return m_device->findSupportedFormat(
      {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint}, vk::ImageTiling::eOptimal,
      vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

} // namespace renderer
} // namespace engine
