#pragma once

#include <SDL3/SDL_video.h>
#include <renderer/vulkan/vulkan_backend.hpp>

class RendererBackend {
public:
  RendererBackend(SDL_Window *window);
  ~RendererBackend();

  bool beginFrame();
  void endFrame();
  void onResize(int width, int height);

private:
  SDL_Window *m_window;
  VulkanBackend m_backend;
  vk::CommandBuffer m_commandBuffer;
};