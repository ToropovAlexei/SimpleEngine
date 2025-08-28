#include "vulkan_renderpass.hpp"
#include <array>

namespace engine::renderer {
VulkanRenderPass::VulkanRenderPass(VulkanDevice device, VkFormat swapchainFormat, VkFormat depthFormat)
  : m_device(device)
{
  VkAttachmentDescription colorAttachment{ .flags = 0,
    .format = swapchainFormat,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR };

  VkAttachmentDescription depthAttachment{
    .flags = 0,
    .format = depthFormat,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
  };

  VkAttachmentReference colorAttachmentRef{ .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

  VkAttachmentReference depthAttachmentRef{ .attachment = 1,
    .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

  VkSubpassDescription subpass{
    .flags = 0,
    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
    .inputAttachmentCount = 0,
    .pInputAttachments = nullptr,
    .colorAttachmentCount = 1,
    .pColorAttachments = &colorAttachmentRef,
    .pResolveAttachments = nullptr,
    .pDepthStencilAttachment = &depthAttachmentRef,
    .preserveAttachmentCount = 0,
    .pPreserveAttachments = nullptr,
  };

  VkSubpassDependency dependency{
    .dependencyFlags = 0,
    .srcSubpass = VK_SUBPASS_EXTERNAL,
    .dstSubpass = 0,
    .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    .srcAccessMask = VK_ACCESS_2_NONE,
    .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
  };

  std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

  VkRenderPassCreateInfo renderPassCreateInfo{
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .attachmentCount = static_cast<uint32_t>(attachments.size()),
    .pAttachments = attachments.data(),
    .subpassCount = 1,
    .pSubpasses = &subpass,
    .dependencyCount = 1,
    .pDependencies = &dependency,
  };

  vkCreateRenderPass(m_device.getDevice(), &renderPassCreateInfo, nullptr, &m_renderPass);
}

VulkanRenderPass::~VulkanRenderPass()
{
  if (m_renderPass) { vkDestroyRenderPass(m_device.getDevice(), m_renderPass, nullptr); }
}
}// namespace engine::renderer
