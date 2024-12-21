#include "renderer.hpp"

Renderer::Renderer(SDL_Window *window) { m_renderer = SDL_CreateRenderer(window, nullptr); }

Renderer::~Renderer() { SDL_DestroyRenderer(m_renderer); }