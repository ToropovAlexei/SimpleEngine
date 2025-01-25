#pragma once

#include <chrono>

namespace engine {
namespace core {
class Timer {
public:
  Timer();

  void tick();
  void reset();

  float getLifeTime();
  float getDeltaTime();

private:
  std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime;
  std::chrono::time_point<std::chrono::high_resolution_clock> m_lastTick;
};
} // namespace core
} // namespace engine
