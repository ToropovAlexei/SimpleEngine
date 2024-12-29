#include "vulkan_renderpass.hpp"
#include <vulkan/vulkan.hpp>

VulkanRenderPass::VulkanRenderPass(VulkanDevice device, vk::Format swapchainFormat, vk::Format depthFormat)
    : m_device(device) {
  vk::AttachmentDescription colorAttachment{.format = swapchainFormat,
                                            .samples = vk::SampleCountFlagBits::e1,
                                            .loadOp = vk::AttachmentLoadOp::eClear,
                                            .storeOp = vk::AttachmentStoreOp::eStore,
                                            .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
                                            .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
                                            .initialLayout = vk::ImageLayout::eUndefined,
                                            .finalLayout = vk::ImageLayout::ePresentSrcKHR};

  vk::AttachmentDescription depthAttachment{
      .format = depthFormat,
      .samples = vk::SampleCountFlagBits::e1,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eDontCare,
      .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
      .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
      .initialLayout = vk::ImageLayout::eUndefined,
      .finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
  };

  vk::AttachmentReference colorAttachmentRef{.attachment = 0, .layout = vk::ImageLayout::eColorAttachmentOptimal};

  vk::AttachmentReference depthAttachmentRef{.attachment = 1,
                                             .layout = vk::ImageLayout::eDepthStencilAttachmentOptimal};

  vk::SubpassDescription subpass{.pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
                                 .colorAttachmentCount = 1,
                                 .pColorAttachments = &colorAttachmentRef,
                                 .pDepthStencilAttachment = &depthAttachmentRef};

  vk::SubpassDependency dependency{
      .srcSubpass = vk::SubpassExternal,
      .dstSubpass = 0,
      .srcStageMask =
          vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
      .dstStageMask =
          vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
      .srcAccessMask = vk::AccessFlagBits::eNone,
      .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite,
  };

  std::array<vk::AttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};

  vk::RenderPassCreateInfo renderPassCreateInfo{.attachmentCount = static_cast<uint32_t>(attachments.size()),
                                                .pAttachments = attachments.data(),
                                                .subpassCount = 1,
                                                .pSubpasses = &subpass,
                                                .dependencyCount = 1,
                                                .pDependencies = &dependency};

  m_renderPass = m_device.getDevice().createRenderPass(renderPassCreateInfo);
}

VulkanRenderPass::~VulkanRenderPass() {
  if (m_renderPass) {
    m_device.getDevice().destroyRenderPass(m_renderPass);
  }
}