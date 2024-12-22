#pragma once

class RendererBackend {
public:
  RendererBackend();
  ~RendererBackend();

  bool beginFrame();
  void endFrame();

private:
};