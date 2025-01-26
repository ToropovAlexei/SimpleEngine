#include "test_renderer.hpp"

TestRenderer::TestRenderer(engine::renderer::VulkanRenderer *renderer) : m_renderer{renderer} { createPipeline(); }

TestRenderer::~TestRenderer() {
  // TODO this is wrong
  m_renderer->m_device->flushGPU();
}

void TestRenderer::render(VkCommandBuffer commandBuffer) {
  m_pipeline->bind(commandBuffer);
  // TODO
}

void TestRenderer::createPipeline() {
  engine::renderer::PipelineVkConfigInfo pipelineConfig = {};
  engine::renderer::VulkanPipeline::defaultPipelineVkConfigInfo(pipelineConfig);

  pipelineConfig.vertexInputAttributeDescriptions = Vertex::getAttributeDescriptions();
  pipelineConfig.vertexInputBindingDescriptions = Vertex::getBindingDescriptions();
  m_pipeline = std::make_unique<engine::renderer::VulkanPipeline>(m_renderer->m_device.get(), pipelineConfig);
}