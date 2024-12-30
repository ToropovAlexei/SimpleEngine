#include "renderer_backend.hpp"

bool RendererBackend::beginFrame() {
  m_commandBuffer = m_backend.beginFrame();
  if (!m_commandBuffer) {
    return false;
  }
  m_backend.beginSwapChainRenderPass(m_commandBuffer);
  return true;
}

void RendererBackend::endFrame() {
  m_backend.endSwapChainRenderPass(m_commandBuffer);
  m_backend.endFrame();
}

void RendererBackend::onResize(int width, int height) { m_backend.onResize(width, height); }

RendererBackend::RendererBackend(SDL_Window *window) : m_window{window}, m_backend{window} {}

RendererBackend::~RendererBackend() {}
