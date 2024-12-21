#include "application.hpp"
#include <SDL3/SDL_events.h>

Application::Application(int width, int height, std::string_view name)
    : m_width{width}, m_height{height}, m_window{width, height, name}, m_renderer{m_window.getWindow()} {}

Application::~Application() { SDL_Quit(); }

int Application::run() {
  SDL_Event event;
  while (m_running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        m_running = false;
      }
    }
    m_keyboard.handleEvent(event);
    m_mouse.handleEvent(event);
    m_renderer.clear();
    const double now = ((double)SDL_GetTicks()) / 1000.0; /* convert from milliseconds to seconds. */
    /* choose the color for the frame we will draw. The sine wave trick makes it fade between colors smoothly. */
    const uint8_t red = static_cast<uint8_t>(255 * SDL_sin(now));
    const uint8_t green = static_cast<uint8_t>(255 * SDL_sin(now + SDL_PI_D * 2 / 3));
    const uint8_t blue = static_cast<uint8_t>(255 * SDL_sin(now + SDL_PI_D * 4 / 3));
    m_renderer.setDrawColor(red, green, blue, SDL_ALPHA_OPAQUE);
    m_renderer.present();
  }

  return 0;
}
