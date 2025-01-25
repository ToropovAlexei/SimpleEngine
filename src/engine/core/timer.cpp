#include "timer.hpp"

namespace engine {
namespace core {
using Clock = std::chrono::high_resolution_clock;

Timer::Timer() {
  m_startTime = Clock::now();
  m_lastTick = Clock::now();
}

void Timer::tick() { m_lastTick = Clock::now(); }

void Timer::reset() {
  m_lastTick = Clock::now();
  m_startTime = Clock::now();
}

float Timer::getLifeTime() {
  std::chrono::time_point<Clock> now = Clock::now();
  std::chrono::duration<float> lifeTime = now - m_startTime;
  return lifeTime.count();
}

float Timer::getDeltaTime() {
  std::chrono::time_point<Clock> now = Clock::now();
  std::chrono::duration<float> delta = now - m_lastTick;
  return delta.count();
}
} // namespace core
} // namespace engine
