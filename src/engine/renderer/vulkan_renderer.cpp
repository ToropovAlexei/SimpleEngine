#include "vulkan_renderer.hpp"
#include "SDL3/SDL_video.h"
#include <engine/core/assert.hpp>
#include <engine/core/exception.hpp>
#include <engine/core/logger.hpp>
#include <engine/renderer/vulkan/vulkan_swapchain.hpp>
#include <engine/renderer/vulkan/vulkan_utils.hpp>

namespace engine {
namespace renderer {
VulkanRenderer::VulkanRenderer(SDL_Window *window) : m_window{window} {
  m_device = std::make_unique<VulkanDevice>(m_window);
  recreateSwapChain();
  createCommandBuffers();
}

VulkanRenderer::~VulkanRenderer() {
  m_device->flushGPU();
  vkFreeCommandBuffers(m_device->getDevice(), m_device->getCommandPool(),
                       static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
  m_commandBuffers.clear();
}

void VulkanRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
  SE_ASSERT(m_isFrameStarted, "Can't call beginSwapChainRenderPass "
                              "without first calling beginFrame");
  SE_ASSERT(commandBuffer == getCurrentCommandBuffer(),
            "Can't call beginSwapChainRenderPass on a different command buffer");

  std::array<VkClearValue, 2> clearValues{};
  clearValues[0] = VkClearValue({{{1.0f, 1.0f, 1.0f, 1.0f}}});
  clearValues[1] = VkClearValue({.depthStencil = {1.0f, 0}});

  VkRenderPassBeginInfo renderPassInfo = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .pNext = nullptr,
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

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

  VkViewport viewport{.x = 0.0f,
                      .y = 0.0f,
                      .width = static_cast<float>(m_swapChain->getSwapChainExtent().width),
                      .height = static_cast<float>(m_swapChain->getSwapChainExtent().height),
                      .minDepth = 0.0f,
                      .maxDepth = 1.0f};
  VkRect2D scissor{{0, 0}, m_swapChain->getSwapChainExtent()};
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void VulkanRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
  SE_ASSERT(m_isFrameStarted, "Can't call endSwapChainRenderPass "
                              "without first calling beginFrame");
  SE_ASSERT(commandBuffer == getCurrentCommandBuffer(),
            "Can't call endSwapChainRenderPass on a different command buffer");

  vkCmdEndRenderPass(commandBuffer);
}

VkCommandBuffer VulkanRenderer::beginFrame() {
  SE_ASSERT(!m_isFrameStarted, "Can't call beginFrame while already in progress");

  auto result = m_swapChain->acquireNextImage(&m_currentImageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapChain();
    return nullptr;
  }

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    SE_THROW_ERROR("Failed to acquire swap chain image!");
  }

#ifndef NDEBUG
  m_isFrameStarted = true;
#endif

  auto commandBuffer = getCurrentCommandBuffer();
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo));

  return commandBuffer;
}

void VulkanRenderer::endFrame() {
  SE_ASSERT(m_isFrameStarted, "Can't call endFrame while frame not in progress");

  auto commandBuffer = getCurrentCommandBuffer();
  vkEndCommandBuffer(commandBuffer);

  auto result = m_swapChain->submitCommandBuffers(&commandBuffer, &m_currentImageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    recreateSwapChain();
  } else if (result != VK_SUCCESS) {
    SE_THROW_ERROR("Failed to present swap chain image!");
  }

#ifndef NDEBUG
  m_isFrameStarted = false;
#endif

  m_currentFrameIndex = (m_currentFrameIndex + 1) % VulkanSwapchain::MAX_FRAMES_IN_FLIGHT;
}

void VulkanRenderer::recreateSwapChain() {
  int width = 0;
  int height = 0;
  SDL_GetWindowSize(m_window, &width, &height);
  VkExtent2D extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
  m_device->flushGPU();

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

void VulkanRenderer::createCommandBuffers() {
  m_commandBuffers.resize(VulkanSwapchain::MAX_FRAMES_IN_FLIGHT);

  VkCommandBufferAllocateInfo allocInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .pNext = nullptr,
      .commandPool = m_device->getCommandPool(),
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size()),
  };

  VK_CHECK_RESULT(vkAllocateCommandBuffers(m_device->getDevice(), &allocInfo, m_commandBuffers.data()));
}

void VulkanRenderer::onResize(int width, int height) {
  if (width == 0 || height == 0)
    return;
  m_device->flushGPU();
  recreateSwapChain();
}
} // namespace renderer
} // namespace engine
