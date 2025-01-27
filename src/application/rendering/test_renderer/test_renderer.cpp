#include "test_renderer.hpp"
#include "engine/renderer/descriptors/pipeline_descriptors.hpp"

TestRenderer::TestRenderer(engine::renderer::VulkanRenderer *renderer) : m_renderer{renderer} { createPipeline(); }

TestRenderer::~TestRenderer() {
  // TODO this is wrong
  m_renderer->flushGPU();
}

void TestRenderer::render(VkCommandBuffer commandBuffer) {
  m_renderer->bindPipeline(commandBuffer, m_pipelineId);
  // TODO
}

void TestRenderer::createPipeline() {
  // TODO paths
  m_vertexShaderId = m_renderer->loadVertexShader("test");
  m_fragmentShaderId = m_renderer->loadFragmentShader("test");
  engine::renderer::GraphicsPipelineDesc desc{};
  desc.depthImageFormat = engine::renderer::DepthImageFormat::D32_FLOAT;
  desc.fragmentShaderId = m_fragmentShaderId;
  desc.vertexShaderId = m_vertexShaderId;
  m_pipelineId = m_renderer->createGraphicsPipeline(desc);
}