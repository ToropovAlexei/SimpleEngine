#include "vulkan_backend.hpp"
#include "SDL3/SDL_video.h"
#include <cstdint>

VulkanBackend::VulkanBackend(SDL_Window *window) : m_window{window} {
  m_device = std::make_unique<VulkanDevice>(m_window);
  int width = 0;
  int height = 0;
  SDL_GetWindowSize(m_window, &width, &height);
  m_swapChain = std::make_unique<VulkanSwapchain>(m_device.get(), vk::Extent2D{
                                                                      .width = static_cast<uint32_t>(width),
                                                                      .height = static_cast<uint32_t>(height),
                                                                  });
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