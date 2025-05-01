#include "test_renderer.hpp"
#include "engine/renderer/descriptors/pipeline_descriptors.hpp"
#include "engine/renderer/vulkan/vulkan_buffer_manager.hpp"
#include "glm/fwd.hpp"

struct Vertex {
  float pos[3];
  float color[3];
};

struct PushConstants {
  glm::vec2 offset;
};

TestRenderer::TestRenderer(engine::renderer::VulkanRenderer *renderer) : m_renderer{renderer} {
  createPipeline();
  static std::vector<Vertex> vertices = {
      {{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
      {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
      {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
  };

  engine::renderer::BufferDesc bufferDesc = {
      .usage = engine::renderer::BufferUsage::VERTEX_BUFFER | engine::renderer::BufferUsage::TRANSFER_DESTINATION,
      .cpuAccess = engine::renderer::BufferCPUAccess::ReadOnly,
      .size = vertices.size() * sizeof(Vertex),
  };

  engine::renderer::BufferDesc stagingBufferDesc = {
      .usage = engine::renderer::BufferUsage::TRANSFER_SOURCE,
      .cpuAccess = engine::renderer::BufferCPUAccess::WriteOnly,
      .size = bufferDesc.size,
  };

  m_vertexBufferId = m_renderer->createBuffer(bufferDesc);
  auto stagingBufferId = m_renderer->createBuffer(stagingBufferDesc);
  m_renderer->writeToBuffer(stagingBufferId, vertices.data(), bufferDesc.size);

  VkCommandBuffer commandBuffer = m_renderer->beginSingleTimeCommands();
  m_renderer->copyBuffer(commandBuffer, m_vertexBufferId, 0, stagingBufferId, 0, bufferDesc.size);
  m_renderer->endSingleTimeCommands(commandBuffer);
}

TestRenderer::~TestRenderer() {
  // TODO this is wrong
  m_renderer->flushGPU();
}

void TestRenderer::render(vk::CommandBuffer commandBuffer) {
  m_renderer->bindPipeline(commandBuffer, m_pipelineId);
  // TODO

  PushConstants data = {
      .offset = m_offset,
  };

  m_renderer->setVertexBuffer(commandBuffer, 0, m_vertexBufferId);
  m_renderer->pushConstant(commandBuffer, m_pipelineId, &data, 0, sizeof(PushConstants));
  vkCmdDraw(commandBuffer, 3, 1, 0, 0);
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

void TestRenderer::update(float dt) {
  m_time += dt;
  m_offset = glm::vec2{std::sin(m_time) * 0.75f, std::cos(m_time) * 0.75f};
}