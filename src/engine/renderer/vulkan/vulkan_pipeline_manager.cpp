#include "vulkan_pipeline_manager.hpp"
#include <vulkan/vulkan_enums.hpp>

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

  std::vector<vk::VertexInputBindingDescription2EXT> inputBindingDescriptions;
  std::vector<vk::VertexInputAttributeDescription2EXT> attributeDescriptions;

  if (desc.vertexShaderId != INVALID_ID) {
    auto bindReflection = m_shaderManager->getVertexBindReflection(desc.vertexShaderId);
    inputBindingDescriptions.insert(inputBindingDescriptions.begin(), bindReflection.bindingDescriptions.begin(),
                                    bindReflection.bindingDescriptions.end());
    attributeDescriptions.insert(attributeDescriptions.begin(), bindReflection.attributeDescriptions.begin(),
                                 bindReflection.attributeDescriptions.end());

    for (BindInfoPushConstant &pushConstant : bindReflection.pushConstants) {
      vk::PushConstantRange &range = pipeline.pushConstantRanges.emplace_back();
      range.offset = pushConstant.offset;
      range.size = pushConstant.size;
      range.stageFlags = pushConstant.stageFlags;
    }
  }

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {
      .setLayoutCount = 0,    // TODO
      .pSetLayouts = nullptr, // TODO
      .pushConstantRangeCount = static_cast<uint32_t>(pipeline.pushConstantRanges.size()),
      .pPushConstantRanges = pipeline.pushConstantRanges.data(),
  };

  std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
  if (desc.vertexShaderId != INVALID_ID) {
    vk::PipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;

    vertShaderStageInfo.module = m_shaderManager->getVertexShaderModule(desc.vertexShaderId);
    vertShaderStageInfo.pName = "main";

    shaderStages.push_back(vertShaderStageInfo);
  }
  if (desc.fragmentShaderId != INVALID_ID) {
    vk::PipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;

    fragShaderStageInfo.module = m_shaderManager->getFragmentShaderModule(desc.fragmentShaderId);
    fragShaderStageInfo.pName = "main";

    shaderStages.push_back(fragShaderStageInfo);
  }

  pipeline.pipelineLayout = m_device->getDevice().createPipelineLayout(pipelineLayoutInfo).value;

  vk::PipelineDepthStencilStateCreateInfo depthStencil = {};
  depthStencil.depthTestEnable = desc.depthStencilState.depthEnable;
  depthStencil.depthWriteEnable = desc.depthStencilState.depthWriteEnable;
  depthStencil.depthCompareOp = static_cast<vk::CompareOp>(desc.depthStencilState.depthFunc);
  // depthStencil.depthBoundsTestEnable = desc.states.depthStencilState;
  // depthStencil.minDepthBounds = 0.0f;
  // depthStencil.maxDepthBounds = 1.0f;
  depthStencil.stencilTestEnable = desc.depthStencilState.stencilEnable;

  depthStencil.front = {};
  depthStencil.front.failOp = static_cast<vk::StencilOp>(desc.depthStencilState.frontFace.stencilFailOp);
  depthStencil.front.passOp = static_cast<vk::StencilOp>(desc.depthStencilState.frontFace.stencilPassOp);
  depthStencil.front.depthFailOp = static_cast<vk::StencilOp>(desc.depthStencilState.frontFace.stencilDepthFailOp);
  depthStencil.front.compareOp = static_cast<vk::CompareOp>(desc.depthStencilState.frontFace.stencilFunc);
  // depthStencil.front.compareMask;
  // depthStencil.front.writeMask;
  // depthStencil.front.reference;

  depthStencil.back = {};
  depthStencil.back.failOp = static_cast<vk::StencilOp>(desc.depthStencilState.backFace.stencilFailOp);
  depthStencil.back.passOp = static_cast<vk::StencilOp>(desc.depthStencilState.backFace.stencilPassOp);
  depthStencil.back.depthFailOp = static_cast<vk::StencilOp>(desc.depthStencilState.backFace.stencilDepthFailOp);
  depthStencil.back.compareOp = static_cast<vk::CompareOp>(desc.depthStencilState.backFace.stencilFunc);
  // depthStencil.back.compareMask;
  // depthStencil.back.writeMask;
  // depthStencil.back.reference;

  vk::PipelineViewportStateCreateInfo viewportState = {
      .viewportCount = 1,
      .pViewports = nullptr,
      .scissorCount = 1,
      .pScissors = nullptr,
  };

  vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
  vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(inputBindingDescriptions.size());
  // vertexInputInfo.pVertexBindingDescriptions = inputBindingDescriptions.data();
  vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
  // vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

  vk::PipelineInputAssemblyStateCreateInfo inputAssembly = {
      .topology = static_cast<vk::PrimitiveTopology>(desc.primitiveTopology),
      .primitiveRestartEnable = vk::False,
  };

  auto dynamicStates = std::to_array({
      vk::DynamicState::eViewport,
      vk::DynamicState::eScissor,
      vk::DynamicState::eDepthBias,
  });

  vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
  dynamicStateCreateInfo.setDynamicStates(dynamicStates);

  vk::PipelineRasterizationStateCreateInfo rasterizerState = {
      .depthClampEnable = desc.rasterizerState.depthClampEnabled,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = static_cast<vk::PolygonMode>(desc.rasterizerState.fillMode),
      .cullMode = static_cast<vk::CullModeFlagBits>(desc.rasterizerState.cullMode),
      .frontFace = static_cast<vk::FrontFace>(desc.rasterizerState.frontFaceMode),
      .depthBiasEnable = desc.rasterizerState.depthBiasEnabled,
      .depthBiasConstantFactor = desc.rasterizerState.depthBias,
      .depthBiasClamp = desc.rasterizerState.depthBiasClamp,
      .depthBiasSlopeFactor = desc.rasterizerState.depthBiasSlopeFactor,
      .lineWidth = 1.0f,
  };

  vk::PipelineMultisampleStateCreateInfo multisamplingState = {
      .rasterizationSamples = static_cast<vk::SampleCountFlagBits>(desc.rasterizerState.sampleCount),
      .sampleShadingEnable = VK_FALSE,
      .minSampleShading = 1.0f,          // Optional
      .pSampleMask = nullptr,            // Optional
      .alphaToCoverageEnable = VK_FALSE, // Optional
      .alphaToOneEnable = VK_FALSE       // Optional
  };

  uint32_t numRenderTargets = 1; // TODO : support multiple render targets

  std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments(numRenderTargets);

  for (uint32_t i = 0; i < numRenderTargets; i++) {
    colorBlendAttachments[i].blendEnable = desc.blendState.renderTargets[i].blendEnable;
    colorBlendAttachments[i].srcColorBlendFactor =
        static_cast<vk::BlendFactor>(desc.blendState.renderTargets[i].srcBlend);
    colorBlendAttachments[i].dstColorBlendFactor =
        static_cast<vk::BlendFactor>(desc.blendState.renderTargets[i].destBlend);
    colorBlendAttachments[i].colorBlendOp = static_cast<vk::BlendOp>(desc.blendState.renderTargets[i].blendOp);
    colorBlendAttachments[i].srcAlphaBlendFactor =
        static_cast<vk::BlendFactor>(desc.blendState.renderTargets[i].srcBlendAlpha);
    colorBlendAttachments[i].dstAlphaBlendFactor =
        static_cast<vk::BlendFactor>(desc.blendState.renderTargets[i].destBlendAlpha);
    colorBlendAttachments[i].alphaBlendOp = static_cast<vk::BlendOp>(desc.blendState.renderTargets[i].blendOpAlpha);
    colorBlendAttachments[i].colorWriteMask =
        static_cast<vk::ColorComponentFlags>(desc.blendState.renderTargets[i].renderTargetWriteMask);
  }

  vk::PipelineColorBlendStateCreateInfo colorBlending = {
      .logicOpEnable = desc.blendState.renderTargets[0].logicOpEnable,
      .logicOp = static_cast<vk::LogicOp>(desc.blendState.renderTargets[0].logicOp),
      .attachmentCount = numRenderTargets,
      .pAttachments = colorBlendAttachments.data(),
      .blendConstants = std::array{0.0f, 0.0f, 0.0f, 0.0f},
  };

  vk::Format swapChainFormat = vk::Format(m_swapchain->getSwapChainImageFormat());
  vk::PipelineRenderingCreateInfoKHR pipelineRenderingCreateInfo = {
      .viewMask = 0,
      .colorAttachmentCount = 1,
      .pColorAttachmentFormats = &swapChainFormat,
      .depthAttachmentFormat = static_cast<vk::Format>(desc.depthImageFormat),
      .stencilAttachmentFormat = vk::Format::eUndefined,
  };

  vk::GraphicsPipelineCreateInfo pipelineInfo = {
      .pNext = &pipelineRenderingCreateInfo,
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

  pipeline.pipeline = m_device->getDevice().createGraphicsPipeline(nullptr, pipelineInfo).value;
  m_graphicsPipelines.push_back(pipeline);

  return m_graphicsPipelines.size() - 1;
}
} // namespace renderer
} // namespace engine
