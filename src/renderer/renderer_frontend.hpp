#pragma once

#include <renderer/renderer_backend.hpp>
#include <renderer/renderer_types.hpp>

class RendererFrontend {
public:
  RendererFrontend();
  ~RendererFrontend();

  void onResize(int width, int height);
  void drawFrame(FrameData &frameData);

private:
  bool beginFrame();
  void endFrame();

private:
  RendererBackend m_backend;
};