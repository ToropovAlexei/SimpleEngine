#include "application.hpp"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
// #include "imgui_impl_sdlrenderer3.h"
#include <SDL3/SDL_events.h>
#include <engine/core/assert.hpp>
#include <engine/renderer/renderer_types.hpp>

Application::Application(int width, int height, std::string_view name)
    : m_width{width}, m_height{height}, m_window{width, height, name}, m_renderer{m_window.getWindow()} {
  // IMGUI_CHECKVERSION();
  // ImGui::CreateContext();
  // ImGuiIO &io = ImGui::GetIO();
  // (void)io;
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  // ImGui::StyleColorsDark();
  // ImGui_ImplSDL3_InitForSDLRenderer(m_window.getWindow(), m_renderer.getRenderer());
  // ImGui_ImplSDLRenderer3_Init(m_renderer.getRenderer());
}

Application::~Application() {
  // ImGui_ImplSDLRenderer3_Shutdown();
  // ImGui_ImplSDL3_Shutdown();
  // ImGui::DestroyContext();
  // SDL_Quit();
}

int Application::run() {
  while (m_running) {
    m_clock.update();
    double dt = m_clock.getDeltaTime();
    handleEvents();
    update(dt);
    render(dt);
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
      m_renderer.onResize(m_width, m_height);
    }
    m_keyboard.handleEvent(event);
    m_mouse.handleEvent(event);
  }
}

void Application::update(double dt) {}

void Application::render(double dt) {
  FrameData frameData{.deltaTime = dt};
  if (auto commandBuffer = m_renderer.beginFrame()) {
    m_renderer.beginSwapChainRenderPass(commandBuffer);
    m_renderer.endSwapChainRenderPass(commandBuffer);
    m_renderer.endFrame();
  }

  // ImGui_ImplSDLRenderer3_NewFrame();
  // ImGui_ImplSDL3_NewFrame();
  // ImGui::NewFrame();
  // ImGui::ShowDemoWindow();
  // ImGui::Render();
  const double now = (static_cast<double>(m_clock.getElapsedTime()));
  const uint8_t red = static_cast<uint8_t>(255 * SDL_sin(now));
  const uint8_t green = static_cast<uint8_t>(255 * SDL_sin(now + SDL_PI_D * 2 / 3));
  const uint8_t blue = static_cast<uint8_t>(255 * SDL_sin(now + SDL_PI_D * 4 / 3));
  // m_renderer.setDrawColor(red, green, blue, SDL_ALPHA_OPAQUE);
  // m_renderer.clear();
  // m_renderer.setDrawColor(50 * m_keyboard.isKeyDown(SDL_SCANCODE_LSHIFT), 0, 0, SDL_ALPHA_OPAQUE);
  // m_renderer.renderRect({m_mouse.getMouseX() - 50, m_mouse.getMouseY() - 50, 100, 100});
  // ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_renderer.getRenderer());
  // m_renderer.present();
}
