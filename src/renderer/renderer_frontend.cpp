#include "renderer_frontend.hpp"

void RendererFrontend::drawFrame(FrameData &frameData) {
  if (beginFrame()) {
    endFrame();
  }
}

bool RendererFrontend::beginFrame() { return m_backend.beginFrame(); }

void RendererFrontend::endFrame() { m_backend.endFrame(); }

void RendererFrontend::onResize(int width, int height) {}

RendererFrontend::RendererFrontend(SDL_Window *window) : m_window{window}, m_backend{window} {}

RendererFrontend::~RendererFrontend() {}
