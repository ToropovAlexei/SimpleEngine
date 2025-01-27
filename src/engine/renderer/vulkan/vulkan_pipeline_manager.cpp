#include "vulkan_pipeline_manager.hpp"
#include "engine/renderer/vulkan/vulkan_utils.hpp"

namespace engine {
namespace renderer {
VulkanPipelineManager::VulkanPipelineManager(VulkanDevice *device, VulkanShaderManager *shaderManager,
                                             VulkanSwapchain *swapchain)
    : m_device{device}, m_shaderManager{shaderManager}, m_swapchain{swapchain} {}

VulkanPipelineManager::~VulkanPipelineManager() {
  for (auto &pipeline : m_graphicsPipelines) {
    vkDestroyPipeline(m_device->getDevice(), pipeline.pipeline, nullptr);
    vkDestroyPipelineLayout(m_device->getDevice(), pipeline.pipelineLayout, nullptr);
  }
}

size_t VulkanPipelineManager::createGraphicsPipeline(GraphicsPipelineDesc &desc) {
  GraphicsPipeline pipeline;

  VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .setLayoutCount = 0,            // TODO
      .pSetLayouts = nullptr,         // TODO
      .pushConstantRangeCount = 0,    // TODO
      .pPushConstantRanges = nullptr, // TODO
  };

  std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
  if (desc.vertexShaderId != INVALID_ID) {
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

    vertShaderStageInfo.module = m_shaderManager->getVertexShaderModule(desc.vertexShaderId);
    vertShaderStageInfo.pName = "main";

    shaderStages.push_back(vertShaderStageInfo);
  }
  if (desc.fragmentShaderId != INVALID_ID) {
    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

    fragShaderStageInfo.module = m_shaderManager->getFragmentShaderModule(desc.fragmentShaderId);
    fragShaderStageInfo.pName = "main";

    shaderStages.push_back(fragShaderStageInfo);
  }

  vkCreatePipelineLayout(m_device->getDevice(), &pipelineLayoutInfo, nullptr, &pipeline.pipelineLayout);

  VkPipelineDepthStencilStateCreateInfo depthStencil = {};
  depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencil.depthTestEnable = desc.depthStencilState.depthEnable;
  depthStencil.depthWriteEnable = desc.depthStencilState.depthWriteEnable;
  depthStencil.depthCompareOp = static_cast<VkCompareOp>(desc.depthStencilState.depthFunc);
  // depthStencil.depthBoundsTestEnable = desc.states.depthStencilState;
  // depthStencil.minDepthBounds = 0.0f;
  // depthStencil.maxDepthBounds = 1.0f;
  depthStencil.stencilTestEnable = desc.depthStencilState.stencilEnable;

  depthStencil.front = {};
  depthStencil.front.failOp = static_cast<VkStencilOp>(desc.depthStencilState.frontFace.stencilFailOp);
  depthStencil.front.passOp = static_cast<VkStencilOp>(desc.depthStencilState.frontFace.stencilPassOp);
  depthStencil.front.depthFailOp = static_cast<VkStencilOp>(desc.depthStencilState.frontFace.stencilDepthFailOp);
  depthStencil.front.compareOp = static_cast<VkCompareOp>(desc.depthStencilState.frontFace.stencilFunc);
  // depthStencil.front.compareMask;
  // depthStencil.front.writeMask;
  // depthStencil.front.reference;

  depthStencil.back = {};
  depthStencil.back.failOp = static_cast<VkStencilOp>(desc.depthStencilState.backFace.stencilFailOp);
  depthStencil.back.passOp = static_cast<VkStencilOp>(desc.depthStencilState.backFace.stencilPassOp);
  depthStencil.back.depthFailOp = static_cast<VkStencilOp>(desc.depthStencilState.backFace.stencilDepthFailOp);
  depthStencil.back.compareOp = static_cast<VkCompareOp>(desc.depthStencilState.backFace.stencilFunc);
  // depthStencil.back.compareMask;
  // depthStencil.back.writeMask;
  // depthStencil.back.reference;

  VkPipelineViewportStateCreateInfo viewportState = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .viewportCount = 1,
      .pViewports = nullptr,
      .scissorCount = 1,
      .pScissors = nullptr,
  };

  std::vector<VkVertexInputBindingDescription> inputBindingDescriptions;
  std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

  if (desc.vertexShaderId != INVALID_ID) {
    auto bindReflection = m_shaderManager->getVertexBindReflection(desc.vertexShaderId);
    inputBindingDescriptions.insert(inputBindingDescriptions.begin(), bindReflection.bindingDescriptions.begin(),
                                    bindReflection.bindingDescriptions.end());
    attributeDescriptions.insert(attributeDescriptions.begin(), bindReflection.attributeDescriptions.begin(),
                                 bindReflection.attributeDescriptions.end());
  }

  VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(inputBindingDescriptions.size());
  vertexInputInfo.pVertexBindingDescriptions = inputBindingDescriptions.data();
  vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
  vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

  VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .topology = static_cast<VkPrimitiveTopology>(desc.primitiveTopology),
      .primitiveRestartEnable = VK_FALSE,
  };

  auto dynamicStates = std::to_array({
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR,
      VK_DYNAMIC_STATE_DEPTH_BIAS,
  });

  VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
      .pDynamicStates = dynamicStates.data(),
  };

  VkPipelineRasterizationStateCreateInfo rasterizerState = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .depthClampEnable = desc.rasterizerState.depthClampEnabled,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = static_cast<VkPolygonMode>(desc.rasterizerState.fillMode),
      .cullMode = static_cast<VkCullModeFlags>(desc.rasterizerState.cullMode),
      .frontFace = static_cast<VkFrontFace>(desc.rasterizerState.frontFaceMode),
      .depthBiasEnable = desc.rasterizerState.depthBiasEnabled,
      .depthBiasConstantFactor = desc.rasterizerState.depthBias,
      .depthBiasClamp = desc.rasterizerState.depthBiasClamp,
      .depthBiasSlopeFactor = desc.rasterizerState.depthBiasSlopeFactor,
      .lineWidth = 1.0f,
  };

  VkPipelineMultisampleStateCreateInfo multisamplingState = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .rasterizationSamples = static_cast<VkSampleCountFlagBits>(desc.rasterizerState.sampleCount),
      .sampleShadingEnable = VK_FALSE,
      .minSampleShading = 1.0f,          // Optional
      .pSampleMask = nullptr,            // Optional
      .alphaToCoverageEnable = VK_FALSE, // Optional
      .alphaToOneEnable = VK_FALSE       // Optional
  };

  uint32_t numRenderTargets = 1; // TODO : support multiple render targets

  std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments(numRenderTargets);

  for (uint32_t i = 0; i < numRenderTargets; i++) {
    colorBlendAttachments[i].blendEnable = desc.blendState.renderTargets[i].blendEnable;
    colorBlendAttachments[i].srcColorBlendFactor =
        static_cast<VkBlendFactor>(desc.blendState.renderTargets[i].srcBlend);
    colorBlendAttachments[i].dstColorBlendFactor =
        static_cast<VkBlendFactor>(desc.blendState.renderTargets[i].destBlend);
    colorBlendAttachments[i].colorBlendOp = static_cast<VkBlendOp>(desc.blendState.renderTargets[i].blendOp);
    colorBlendAttachments[i].srcAlphaBlendFactor =
        static_cast<VkBlendFactor>(desc.blendState.renderTargets[i].srcBlendAlpha);
    colorBlendAttachments[i].dstAlphaBlendFactor =
        static_cast<VkBlendFactor>(desc.blendState.renderTargets[i].destBlendAlpha);
    colorBlendAttachments[i].alphaBlendOp = static_cast<VkBlendOp>(desc.blendState.renderTargets[i].blendOpAlpha);
    colorBlendAttachments[i].colorWriteMask =
        static_cast<VkColorComponentFlags>(desc.blendState.renderTargets[i].renderTargetWriteMask);
  }

  VkPipelineColorBlendStateCreateInfo colorBlending = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .logicOpEnable = desc.blendState.renderTargets[0].logicOpEnable,
      .logicOp = static_cast<VkLogicOp>(desc.blendState.renderTargets[0].logicOp),
      .attachmentCount = numRenderTargets,
      .pAttachments = colorBlendAttachments.data(),
      .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},
  };

  VkFormat swapChainFormat = m_swapchain->getSwapChainImageFormat();
  VkPipelineRenderingCreateInfoKHR pipelineRenderingCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
      .pNext = nullptr,
      .viewMask = 0,
      .colorAttachmentCount = 1,
      .pColorAttachmentFormats = &swapChainFormat,
      .depthAttachmentFormat = static_cast<VkFormat>(desc.depthImageFormat),
      .stencilAttachmentFormat = VK_FORMAT_UNDEFINED,
  };

  VkGraphicsPipelineCreateInfo pipelineInfo = {
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .pNext = &pipelineRenderingCreateInfo,
      .flags = 0,
      .stageCount = static_cast<uint32_t>(shaderStages.size()),
      .pStages = shaderStages.data(),
      .pVertexInputState = &vertexInputInfo,
      .pInputAssemblyState = &inputAssembly,
      .pTessellationState = nullptr,
      .pViewportState = &viewportState,
      .pRasterizationState = &rasterizerState,
      .pMultisampleState = &multisamplingState,
      .pDepthStencilState = &depthStencil,
      .pColorBlendState = &colorBlending,
      .pDynamicState = &dynamicStateCreateInfo,
      .layout = pipeline.pipelineLayout,
      .renderPass = nullptr, // Maybe not
      .subpass = 0,
      .basePipelineHandle = VK_NULL_HANDLE,
      .basePipelineIndex = -1,
  };

  VK_CHECK_RESULT(
      vkCreateGraphicsPipelines(m_device->getDevice(), nullptr, 1, &pipelineInfo, nullptr, &pipeline.pipeline));
  m_graphicsPipelines.push_back(pipeline);

  return m_graphicsPipelines.size() - 1;
}
} // namespace renderer
} // namespace engine