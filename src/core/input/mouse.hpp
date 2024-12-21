#pragma once

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
#include <bitset>

class Mouse {
  enum class Button {
    LEFT = SDL_BUTTON_LEFT,
    MIDDLE = SDL_BUTTON_MIDDLE,
    RIGHT = SDL_BUTTON_RIGHT,
    X1 = SDL_BUTTON_X1,
    X2 = SDL_BUTTON_X2,
    COUNT
  };

public:
  void handleEvent(const SDL_Event &event) noexcept {
    if (event.type == SDL_EVENT_MOUSE_MOTION) {
      m_X = event.motion.x;
      m_Y = event.motion.y;
      m_dX = event.motion.xrel;
      m_dY = event.motion.yrel;
    } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
      m_pressedBtns[event.button.button] = event.type == SDL_EVENT_MOUSE_BUTTON_DOWN;
    }
  }

  inline bool isButtonPressed(Button button) const noexcept { return m_pressedBtns[static_cast<size_t>(button)]; }
  inline bool isButtonReleased(Button button) const noexcept { return !m_pressedBtns[static_cast<size_t>(button)]; }
  inline float getMouseX() const noexcept { return m_X; }
  inline float getMouseY() const noexcept { return m_Y; }
  inline float getMouseDeltaX() const noexcept { return m_dX; }
  inline float getMouseDeltaY() const noexcept { return m_dY; }

private:
  float m_X = 0;
  float m_Y = 0;
  float m_dX = 0;
  float m_dY = 0;
  std::bitset<static_cast<size_t>(Button::COUNT)> m_pressedBtns;
};