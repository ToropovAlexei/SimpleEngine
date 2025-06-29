#pragma once
#include "application/rendering/test_renderer/gl_test_renderer.hpp"
#include "engine/renderer/gl_renderer.hpp"
#include <engine/core/window.hpp>
#include <memory>

class GameRenderer {
public:
  GameRenderer(engine::core::Window &window);

  void render(float dt);

  void updateRenderers(float dt);

  void setRenderSize(int width, int height);
  void setView(const glm::mat4 &view) {
    m_view = view;
    m_testRenderer->setView(view);
  }
  void setProjection(const glm::mat4 &projection) {
    m_projection = projection;
    m_testRenderer->setProjection(projection);
  }
  void setCameraPos(const glm::vec3 &pos) { m_testRenderer->setCameraPos(pos); }

private:
  engine::core::Window &m_window;
  std::unique_ptr<engine::renderer::GlRenderer> m_renderer;
  std::unique_ptr<GlTestRenderer> m_testRenderer;
  glm::mat4 m_view;
  glm::mat4 m_projection;
  // std::unique_ptr<engine::renderer::VulkanDescriptorPool> m_globalPool;
  // GlobalUBO m_ubo;
  // std::vector<VkDescriptorSet> m_globalDescriptorSets;
  // std::vector<size_t> m_globalBufferIds;
  // std::unique_ptr<TestRenderer> m_testRenderer;
};
