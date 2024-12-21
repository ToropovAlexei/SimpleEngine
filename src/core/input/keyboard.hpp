#pragma once

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_scancode.h>
#include <bitset>

class Keyboard {
public:
  inline void handleEvent(const SDL_Event &event) noexcept {
    if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
      m_scancodes[event.key.scancode] = event.type == SDL_EVENT_KEY_DOWN;
    }
  }
  inline bool isKeyDown(SDL_Scancode scancode) const noexcept { return m_scancodes[scancode]; }
  inline bool isKeyUp(SDL_Scancode scancode) const noexcept { return !m_scancodes[scancode]; }
  inline void clear() noexcept { m_scancodes.reset(); }

private:
  std::bitset<SDL_SCANCODE_COUNT> m_scancodes;
};