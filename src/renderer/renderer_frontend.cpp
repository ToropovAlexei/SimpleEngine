#include "renderer_frontend.hpp"

void RendererFrontend::drawFrame(FrameData &frameData) {
  if (beginFrame()) {
    endFrame();
  }
}

bool RendererFrontend::beginFrame() { return m_backend.beginFrame(); }

void RendererFrontend::endFrame() { m_backend.endFrame(); }

void RendererFrontend::onResize(int width, int height) {}

RendererFrontend::RendererFrontend() {}

RendererFrontend::~RendererFrontend() {}
