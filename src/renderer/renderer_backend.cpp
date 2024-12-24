#include "renderer_backend.hpp"

bool RendererBackend::beginFrame() { return true; }

void RendererBackend::endFrame() {}

RendererBackend::RendererBackend(SDL_Window *window) : m_window{window}, m_backend{window} {}

RendererBackend::~RendererBackend() {}
