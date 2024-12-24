#pragma once

#include <SDL3/SDL_video.h>
#include <renderer/renderer_backend.hpp>
#include <renderer/renderer_types.hpp>

class RendererFrontend {
public:
  RendererFrontend(SDL_Window *window);
  ~RendererFrontend();

  void onResize(int width, int height);
  void drawFrame(FrameData &frameData);

private:
  bool beginFrame();
  void endFrame();

private:
  SDL_Window *m_window;
  RendererBackend m_backend;
};