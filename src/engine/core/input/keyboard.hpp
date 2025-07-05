#pragma once

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_scancode.h>
#include <algorithm>
#include <bitset>
#include <engine/core/logger.hpp>
#include <functional>
#include <unordered_map>

namespace engine {
namespace core {
class Keyboard {
public:
  struct CallbackId {
    size_t value;
  };
  enum class Key {
    LEFT = SDL_SCANCODE_LEFT,
    RIGHT = SDL_SCANCODE_RIGHT,
    UP = SDL_SCANCODE_UP,
    DOWN = SDL_SCANCODE_DOWN,
    SPACE = SDL_SCANCODE_SPACE,
    ESCAPE = SDL_SCANCODE_ESCAPE,
    W = SDL_SCANCODE_W,
    A = SDL_SCANCODE_A,
    S = SDL_SCANCODE_S,
    D = SDL_SCANCODE_D,
    F = SDL_SCANCODE_F,
    X = SDL_SCANCODE_X
  };

public:
#ifndef NDEBUG
  ~Keyboard() {
    if (std::any_of(m_callbacks.begin(), m_callbacks.end(), [](const auto &pair) { return !pair.empty(); })) {
      LOG_WARN("Keyboard has {} callbacks left",
               std::count_if(m_callbacks.begin(), m_callbacks.end(), [](const auto &pair) { return !pair.empty(); }));
    }
  }
#endif

  inline void handleEvent(const SDL_Event &event) noexcept {
    if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
      if (!m_pressedKeys[event.key.scancode] && event.type == SDL_EVENT_KEY_DOWN) {
        for (const auto &pair : m_callbacks[event.key.scancode]) {
          pair.second();
        }
      }
      m_pressedKeys[event.key.scancode] = event.type == SDL_EVENT_KEY_DOWN;
    }
  }
  inline bool isKeyDown(Key scancode) const noexcept { return m_pressedKeys[static_cast<SDL_Scancode>(scancode)]; }
  inline bool isKeyUp(Key scancode) const noexcept { return !m_pressedKeys[static_cast<SDL_Scancode>(scancode)]; }
  inline void clear() noexcept { m_pressedKeys.reset(); }

  [[nodiscard]] CallbackId onKeyDown(Key scancode, std::function<void()> callback) {
    m_callbacks[static_cast<SDL_Scancode>(scancode)].emplace(nextCallbackId, callback);
    return {nextCallbackId++};
  }

  void unsubscribe(CallbackId id) { m_callbacks[id.value].erase(id.value); }

private:
  std::bitset<SDL_SCANCODE_COUNT> m_pressedKeys;
  std::array<std::unordered_map<size_t, std::function<void()>>, SDL_SCANCODE_COUNT> m_callbacks;
  size_t nextCallbackId = 0;
};
} // namespace core
} // namespace engine