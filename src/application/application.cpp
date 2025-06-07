#include "application.hpp"
#include "imgui_impl_sdl3.h"
#include <SDL3/SDL_events.h>
#include <engine/core/assert.hpp>
#include <engine/renderer/renderer_types.hpp>

Application::Application(int width, int height, std::string_view name)
    : m_width{width}, m_height{height}, m_window{width, height, name}, m_gameRenderer{m_window} {}

Application::~Application() {
  // SDL_Quit();
}

int Application::run() {
  while (m_running) {
    float deltaTime = m_timer.getDeltaTime();
    m_timer.tick();
    handleEvents();
    update(deltaTime);
    render(deltaTime);
  }

  return 0;
}

void Application::handleEvents() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    // ImGui_ImplSDL3_ProcessEvent(&event);
    if (event.type == SDL_EVENT_QUIT) {
      m_running = false;
    }
    if (event.type == SDL_EVENT_WINDOW_RESIZED) {
      m_width = event.window.data1;
      m_height = event.window.data2;
      m_gameRenderer.setRenderSize(m_width, m_height);
    }
    m_keyboard.handleEvent(event);
    m_mouse.handleEvent(event);
  }
}

void Application::update(float dt) { m_gameRenderer.updateRenderers(dt); }

void Application::render(float dt) { m_gameRenderer.render(dt); }
