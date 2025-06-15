#include "game_renderer.hpp"

GameRenderer::GameRenderer(engine::core::Window &window) : m_window{window} {
  m_renderer = std::make_unique<engine::renderer::GlRenderer>(m_window.getWindow());
  m_testRenderer = std::make_unique<GlTestRenderer>(m_renderer.get());
  m_testRenderer->resize(m_window.getWidth(), m_window.getHeight());
}

void GameRenderer::render(float dt) {
  m_renderer->beginFrame();
  m_testRenderer->render();
  m_renderer->endFrame();
}

void GameRenderer::setRenderSize(int width, int height) {
  m_renderer->onResize(width, height);
  m_testRenderer->resize(width, height);
}

void GameRenderer::updateRenderers(float dt) { m_testRenderer->update(dt); }
