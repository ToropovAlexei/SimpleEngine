#include "vulkan_renderer.hpp"
#include "SDL3/SDL_video.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"
#include <engine/core/assert.hpp>
#include <engine/core/exception.hpp>
#include <engine/core/logger.hpp>
#include <engine/renderer/vulkan/vulkan_swapchain.hpp>
#include <engine/renderer/vulkan/vulkan_utils.hpp>
#include <memory>
#include <vulkan/vulkan_core.h>

namespace engine {
namespace renderer {
VulkanRenderer::VulkanRenderer(SDL_Window *window) : m_window{window} {
  m_device = std::make_unique<VulkanDevice>(m_window);
  m_shaderManager = new VulkanShaderManager(m_device.get());
  recreateSwapChain();
  m_pipelineManager = new VulkanPipelineManager(m_device.get(), m_shaderManager, m_swapChain.get());
  m_bufferManager = std::make_unique<VulkanBufferManager>(m_device.get());
  createCommandBuffers();
  initImGui();
}

VulkanRenderer::~VulkanRenderer() {
  m_device->flushGPU();
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
  delete m_shaderManager;
  delete m_pipelineManager;
  vkFreeCommandBuffers(m_device->getDevice(), m_device->getCommandPool(),
                       static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
  vkDestroyDescriptorPool(m_device->getDevice(), m_imguiPool, nullptr);
  m_commandBuffers.clear();
}

void VulkanRenderer::beginRendering(VkCommandBuffer commandBuffer) {
  SE_ASSERT(m_isFrameStarted, "Can't call beginSwapChainRenderPass "
                              "without first calling beginFrame");
  SE_ASSERT(commandBuffer == getCurrentCommandBuffer(),
            "Can't call beginSwapChainRenderPass on a different command buffer");

  m_device->transitionImageLayout(
      commandBuffer, m_swapChain->getImage(m_currentImageIndex), 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
  m_device->transitionImageLayout(
      commandBuffer, m_swapChain->getDepthImage(m_currentImageIndex), 0, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
      VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
      VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
      VkImageSubresourceRange{VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1});

  VkRenderingAttachmentInfoKHR colorAttachment{};
  colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
  colorAttachment.imageView = m_swapChain->getImageView(m_currentImageIndex);
  colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.clearValue.color = {{1.0f, 1.0f, 1.0f, 1.0f}};

  // A single depth stencil attachment info can be used, but they can also be specified separately.
  // When both are specified separately, the only requirement is that the image view is identical.
  VkRenderingAttachmentInfoKHR depthStencilAttachment{};
  depthStencilAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
  depthStencilAttachment.imageView = m_swapChain->getDepthImageView(m_currentImageIndex);
  depthStencilAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  depthStencilAttachment.clearValue.depthStencil = {1.0f, 0};

  VkRenderingInfoKHR renderingInfo{};
  renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
  renderingInfo.renderArea = {{0, 0},
                              {m_swapChain->getSwapChainExtent().width, m_swapChain->getSwapChainExtent().height}};
  renderingInfo.layerCount = 1;
  renderingInfo.colorAttachmentCount = 1;
  renderingInfo.pColorAttachments = &colorAttachment;
  renderingInfo.pDepthAttachment = &depthStencilAttachment;
  renderingInfo.pStencilAttachment = nullptr;

  vkCmdBeginRendering(commandBuffer, &renderingInfo);

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

void VulkanRenderer::endRendering(VkCommandBuffer commandBuffer) {
  SE_ASSERT(m_isFrameStarted, "Can't call endSwapChainRenderPass "
                              "without first calling beginFrame");
  SE_ASSERT(commandBuffer == getCurrentCommandBuffer(),
            "Can't call endSwapChainRenderPass on a different command buffer");

  vkCmdEndRendering(commandBuffer);

  m_device->transitionImageLayout(commandBuffer, m_swapChain->getImage(m_currentImageIndex),
                                  VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                  VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                  VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                                  VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
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

void VulkanRenderer::initImGui() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui::StyleColorsDark();
  VkDescriptorPoolSize poolSizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
                                      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                                      {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
                                      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
                                      {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                                      {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                                      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                                      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
                                      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                                      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                                      {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

  VkDescriptorPoolCreateInfo poolInfo = {};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  poolInfo.maxSets = 1000 * sizeof(poolSizes) / sizeof(poolSizes[0]);
  poolInfo.poolSizeCount = static_cast<uint32_t>(sizeof(poolSizes) / sizeof(poolSizes[0]));
  poolInfo.pPoolSizes = poolSizes;

  vkCreateDescriptorPool(m_device->getDevice(), &poolInfo, nullptr, &m_imguiPool);

  ImGui_ImplSDL3_InitForVulkan(m_window);

  auto imageFormat = m_swapChain->getSwapChainImageFormat();
  VkPipelineRenderingCreateInfoKHR pipelineRenderingCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
      .pNext = nullptr,
      .viewMask = 0,
      .colorAttachmentCount = 1,
      .pColorAttachmentFormats = &imageFormat,
      .depthAttachmentFormat = m_swapChain->findDepthFormat(),
      .stencilAttachmentFormat = VK_FORMAT_UNDEFINED,
  };
  ImGui_ImplVulkan_InitInfo initInfo = {};
  initInfo.Instance = m_device->getInstance();
  initInfo.PhysicalDevice = m_device->getPhysicalDevice();
  initInfo.Device = m_device->getDevice();
  initInfo.Queue = m_device->getGraphicsQueue();
  initInfo.DescriptorPool = m_imguiPool;
  initInfo.UseDynamicRendering = true;
  initInfo.MinImageCount = VulkanSwapchain::MAX_FRAMES_IN_FLIGHT;
  initInfo.ImageCount = VulkanSwapchain::MAX_FRAMES_IN_FLIGHT;
  initInfo.PipelineRenderingCreateInfo = pipelineRenderingCreateInfo;

  ImGui_ImplVulkan_Init(&initInfo);
}

size_t VulkanRenderer::loadFragmentShader(std::string_view path) {
  return m_shaderManager->loadShader(path, VulkanShaderManager::ShaderType::Fragment);
}

size_t VulkanRenderer::loadVertexShader(std::string_view path) {
  return m_shaderManager->loadShader(path, VulkanShaderManager::ShaderType::Vertex);
}

void VulkanRenderer::flushGPU() { m_device->flushGPU(); }

size_t VulkanRenderer::createGraphicsPipeline(GraphicsPipelineDesc &desc) {
  return m_pipelineManager->createGraphicsPipeline(desc);
}

void VulkanRenderer::bindPipeline(VkCommandBuffer commandBuffer, size_t pipelineId) {
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager->getGraphicsPipeline(pipelineId));
}

void VulkanRenderer::draw(VkCommandBuffer commandBuffer, uint32_t numVertices, uint32_t numInstances,
                          uint32_t vertexOffset, uint32_t instanceOffset) {
  vkCmdDraw(commandBuffer, numVertices, numInstances, vertexOffset, instanceOffset);
}

void VulkanRenderer::setVertexBuffer(VkCommandBuffer commandBuffer, uint32_t slot, size_t bufferId) {
  VkBuffer vertexBuffer = m_bufferManager->getBuffer(bufferId);
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, slot, 1, &vertexBuffer, offsets);
}

void VulkanRenderer::setIndexBuffer(VkCommandBuffer commandBuffer, size_t bufferId, IndexFormat indexFormat) {
  VkBuffer indexBuffer = m_bufferManager->getBuffer(bufferId);
  vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, static_cast<VkIndexType>(indexFormat));
}

size_t VulkanRenderer::createBuffer(BufferDesc &desc) { return m_bufferManager->createBuffer(desc); }

void VulkanRenderer::copyBuffer(VkCommandBuffer commandBuffer, size_t dstBuffer, uint64_t dstOffset, size_t srcBuffer,
                                uint64_t srcOffset, uint64_t range) {
  VkBuffer vkDstBuffer = m_bufferManager->getBuffer(dstBuffer);
  VkBuffer vkSrcBuffer = m_bufferManager->getBuffer(srcBuffer);

  VkBufferCopy copyRegion = {
      .srcOffset = srcOffset,
      .dstOffset = dstOffset,
      .size = range,
  };

  size_t srcBufferSize = m_bufferManager->getBufferSize(srcBuffer);
  size_t dstBufferSize = m_bufferManager->getBufferSize(dstBuffer);

  SE_ASSERT(srcOffset + range <= srcBufferSize, "Source Buffer out of bounds!");
  SE_ASSERT(dstOffset + range <= dstBufferSize, "Destination Buffer out of bounds!");

  vkCmdCopyBuffer(commandBuffer, vkSrcBuffer, vkDstBuffer, 1, &copyRegion);
}

// Temporary
void VulkanRenderer::writeToBuffer(size_t bufferId, void *data, VkDeviceSize size) {
  vmaCopyMemoryToAllocation(m_device->getAllocator(), data, m_bufferManager->getBufferAllocation(bufferId), 0, size);
}
// Temporary
VkCommandBuffer VulkanRenderer::beginSingleTimeCommands() { return m_device->beginSingleTimeCommands(); }
// Temporary
void VulkanRenderer::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
  m_device->endSingleTimeCommands(commandBuffer);
}

} // namespace renderer
} // namespace engine
