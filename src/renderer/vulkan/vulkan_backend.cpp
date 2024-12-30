#include "vulkan_backend.hpp"
#include "SDL3/SDL_video.h"
#include "core/logger.hpp"
#include "renderer/vulkan/vulkan_swapchain.hpp"
#include <core/assert.hpp>
#include <core/exception.hpp>
#include <cstdint>
#include <vulkan/vulkan_structs.hpp>

VulkanBackend::VulkanBackend(SDL_Window *window) : m_window{window} {
  m_device = std::make_unique<VulkanDevice>(m_window);
  recreateSwapChain();
  createCommandBuffers();
}

VulkanBackend::~VulkanBackend() {
  m_device->getDevice().waitIdle();
  m_device->getDevice().freeCommandBuffers(m_device->getCommandPool(), m_commandBuffers);
  m_commandBuffers.clear();
}

void VulkanBackend::beginSwapChainRenderPass(vk::CommandBuffer commandBuffer) {
  assert(m_isFrameStarted && "Can't call beginSwapChainRenderPass "
                             "without first calling beginFrame");
  assert(commandBuffer == getCurrentCommandBuffer() &&
         "Can't call beginSwapChainRenderPass on a different command buffer");

  std::array<vk::ClearValue, 2> clearValues{};
  clearValues[0] = vk::ClearValue(vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}));
  clearValues[1] = vk::ClearValue({.depthStencil = {1.0f, 0}});

  vk::RenderPassBeginInfo renderPassInfo = {
      .renderPass = m_swapChain->getRenderPass(),
      .framebuffer = m_swapChain->getFrameBuffer(m_currentImageIndex),
      .renderArea =
          {
              .offset = {0, 0},
              .extent = m_swapChain->getSwapChainExtent(),
          },
      .clearValueCount = static_cast<uint32_t>(clearValues.size()),
      .pClearValues = clearValues.data(),
  };

  commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

  vk::Viewport viewport{.x = 0.0f,
                        .y = 0.0f,
                        .width = static_cast<float>(m_swapChain->getSwapChainExtent().width),
                        .height = static_cast<float>(m_swapChain->getSwapChainExtent().height),
                        .minDepth = 0.0f,
                        .maxDepth = 1.0f};
  vk::Rect2D scissor{{0, 0}, m_swapChain->getSwapChainExtent()};
  commandBuffer.setViewport(0, viewport);
  commandBuffer.setScissor(0, scissor);
}

void VulkanBackend::endSwapChainRenderPass(vk::CommandBuffer commandBuffer) {
  assert(m_isFrameStarted && "Can't call endSwapChainRenderPass "
                             "without first calling beginFrame");
  assert(commandBuffer == getCurrentCommandBuffer() &&
         "Can't call endSwapChainRenderPass on a different command buffer");

  commandBuffer.endRenderPass();
}

vk::CommandBuffer VulkanBackend::beginFrame() {
  SE_ASSERT(!m_isFrameStarted, "Can't call beginFrame while already in progress");

  auto result = m_swapChain->acquireNextImage(&m_currentImageIndex);

  if (result == vk::Result::eErrorOutOfDateKHR) {
    recreateSwapChain();
    return nullptr;
  }

  if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
    SE_THROW_ERROR("Failed to acquire swap chain image!");
  }

#ifndef NDEBUG
  m_isFrameStarted = true;
#endif

  auto commandBuffer = getCurrentCommandBuffer();
  vk::CommandBufferBeginInfo beginInfo{};

  commandBuffer.begin(beginInfo);

  return commandBuffer;
}

void VulkanBackend::endFrame() {
  assert(m_isFrameStarted && "Can't call endFrame while frame not in progress");

  auto commandBuffer = getCurrentCommandBuffer();
  commandBuffer.end();

  auto result = m_swapChain->submitCommandBuffers(&commandBuffer, &m_currentImageIndex);

  if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
    recreateSwapChain();
  } else if (result != vk::Result::eSuccess) {
    SE_THROW_ERROR("Failed to present swap chain image!");
  }

#ifndef NDEBUG
  m_isFrameStarted = false;
#endif

  m_currentFrameIndex = (m_currentFrameIndex + 1) % VulkanSwapchain::MAX_FRAMES_IN_FLIGHT;
}

void VulkanBackend::recreateSwapChain() {
  int width = 0;
  int height = 0;
  SDL_GetWindowSize(m_window, &width, &height);
  vk::Extent2D extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
  m_device->getDevice().waitIdle();

  if (m_swapChain == nullptr) {
    m_swapChain = std::make_unique<VulkanSwapchain>(m_device.get(), extent);
  } else {
    std::shared_ptr<VulkanSwapchain> oldSwapChain = std::move(m_swapChain);
    m_swapChain = std::make_unique<VulkanSwapchain>(m_device.get(), extent, oldSwapChain);

    if (!oldSwapChain->compareSwapFormats(*m_swapChain.get())) {
      SE_THROW_ERROR("Swap chain image or depth format has changed!");
    }
  }
  LOG_INFO("Swap chain recreated");
}

void VulkanBackend::createCommandBuffers() {
  m_commandBuffers.resize(VulkanSwapchain::MAX_FRAMES_IN_FLIGHT);

  vk::CommandBufferAllocateInfo allocInfo = {
      .commandPool = m_device->getCommandPool(),
      .level = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size()),
  };

  m_commandBuffers = m_device->getDevice().allocateCommandBuffers(allocInfo);
}

void VulkanBackend::onResize(int width, int height) {
  if (width == 0 || height == 0)
    return;
  m_device->getDevice().waitIdle();
  recreateSwapChain();
}