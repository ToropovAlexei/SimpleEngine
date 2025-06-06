#pragma once

#include <engine/renderer/vulkan/vulkan_pipeline.hpp>
#include <engine/renderer/vulkan_renderer.hpp>
#include <glm/glm.hpp>

class TestRenderer {
public:
  TestRenderer(engine::renderer::VulkanRenderer *device);
  ~TestRenderer();

  void render(vk::CommandBuffer commandBuffer);
  void update(float dt);

private:
  void createPipeline();

private:
  engine::renderer::VulkanRenderer *m_renderer;
  engine::renderer::ShaderProgramId m_shaderProgram;

  float m_time = 0.0f;
  glm::vec2 m_offset;

  size_t m_vertexBufferId;

  size_t m_vertexShaderId;
  size_t m_fragmentShaderId;
};