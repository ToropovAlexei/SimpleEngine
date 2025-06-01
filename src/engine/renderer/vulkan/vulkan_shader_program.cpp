#include "vulkan_shader_program.hpp"
#include <vector>
#include <vulkan/vulkan.hpp>

namespace engine::renderer {
VulkanShaderProgram::VulkanShaderProgram(VulkanDevice *device, VulkanShaderProgramDesc const &desc)
    : m_device{device}, m_attributes{desc.attributes}, m_bindings{desc.bindings} {
  std::vector<vk::ShaderCreateInfoEXT> infos;
  std::vector<vk::PushConstantRange> pushConstantRanges;

  for (const BindInfoPushConstant &pushConstant : desc.pushConstants) {
    vk::PushConstantRange &range = pushConstantRanges.emplace_back();
    range.offset = pushConstant.offset;
    range.size = pushConstant.size;
    range.stageFlags = pushConstant.stageFlags;
  }

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {
      .setLayoutCount = 0,    // TODO
      .pSetLayouts = nullptr, // TODO
      .pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size()),
      .pPushConstantRanges = pushConstantRanges.data(),
  };

  m_pipelineLayout = m_device->getDevice().createPipelineLayout(pipelineLayoutInfo).value;

  if (desc.vertexSpirv.has_value()) {
    infos.push_back(createShaderCreateInfo(desc.vertexSpirv.value(), desc));
    m_stages.push_back(vk::ShaderStageFlagBits::eVertex);
    infos[0].setPushConstantRanges(pushConstantRanges);
    if (desc.fragmentSpirv.has_value()) {
      infos[0].setStage(vk::ShaderStageFlagBits::eVertex);
      infos[0].setNextStage(vk::ShaderStageFlagBits::eFragment);
      infos[0].setFlags(vk::ShaderCreateFlagBitsEXT::eLinkStage);
    }
  }

  if (desc.fragmentSpirv.has_value()) {
    m_stages.push_back(vk::ShaderStageFlagBits::eFragment);
    infos.push_back(createShaderCreateInfo(desc.fragmentSpirv.value(), desc));
    const size_t idx = infos.size() - 1;
    infos[idx].setStage(vk::ShaderStageFlagBits::eFragment);
    infos[idx].setFlags(vk::ShaderCreateFlagBitsEXT::eLinkStage);
    infos[idx].setPushConstantRanges(pushConstantRanges);
  }

  m_shaders = m_device->getDevice().createShadersEXT(infos).value;
}

VulkanShaderProgram::~VulkanShaderProgram() {
  for (auto shader : m_shaders) {
    m_device->getDevice().destroyShaderEXT(shader);
  }
  m_device->getDevice().destroyPipelineLayout(m_pipelineLayout);
}

vk::ShaderCreateInfoEXT VulkanShaderProgram::createShaderCreateInfo(const std::vector<char> &code,
                                                                    VulkanShaderProgramDesc const &desc) const {
  auto info = vk::ShaderCreateInfoEXT{};
  info.setCodeSize(code.size())
      .setPCode(code.data())
      .setSetLayouts(desc.setLayouts)
      .setCodeType(vk::ShaderCodeTypeEXT::eSpirv)
      .setPName("main");

  return info;
}

void VulkanShaderProgram::bind(vk::CommandBuffer commandBuffer) {
  commandBuffer.setCullMode(vk::CullModeFlagBits::eNone);
  commandBuffer.setFrontFace(vk::FrontFace::eCounterClockwise);
  commandBuffer.setDepthTestEnable(vk::True);
  commandBuffer.setDepthWriteEnable(vk::True);
  commandBuffer.setDepthCompareOp(vk::CompareOp::eLessOrEqual);
  commandBuffer.setPrimitiveTopology(vk::PrimitiveTopology::eTriangleList);
  commandBuffer.setRasterizerDiscardEnable(vk::False);
  commandBuffer.setPolygonModeEXT(vk::PolygonMode::eFill);
  commandBuffer.setRasterizationSamplesEXT(vk::SampleCountFlagBits::e1);
  commandBuffer.setAlphaToCoverageEnableEXT(vk::False);
  commandBuffer.setDepthBiasEnable(vk::False);
  commandBuffer.setStencilTestEnable(vk::False);
  commandBuffer.setPrimitiveRestartEnable(vk::False);
  commandBuffer.setDepthClampEnableEXT(vk::False);

  commandBuffer.setSampleMaskEXT(vk::SampleCountFlagBits::e1, 0xff);

  commandBuffer.setColorBlendEnableEXT(0, {vk::False});
  commandBuffer.setColorWriteMaskEXT(0, vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                            vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

  commandBuffer.setVertexInputEXT(m_bindings, m_attributes);

  commandBuffer.bindShadersEXT(m_stages, m_shaders);
}
} // namespace engine::renderer