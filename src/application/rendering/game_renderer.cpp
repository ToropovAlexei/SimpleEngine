#include "game_renderer.hpp"

GameRenderer::GameRenderer(engine::core::Window &window) : m_window{window} {
  m_renderer = std::make_unique<engine::renderer::GlRenderer>(m_window.getWindow());
  m_testRenderer = std::make_unique<GlTestRenderer>(m_renderer.get());
}

void GameRenderer::render(float dt) {
  m_renderer->beginFrame();
  m_testRenderer->render();
  m_renderer->endFrame();
}

void GameRenderer::setRenderSize(int width, int height) { m_renderer->onResize(width, height); }

void GameRenderer::updateRenderers(float dt) { m_testRenderer->update(dt); }
