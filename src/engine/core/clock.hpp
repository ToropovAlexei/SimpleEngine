#pragma once

#include <chrono>

namespace engine {
namespace core {
class Clock {
public:
  Clock() { reset(); }

  void reset() {
    startTime = std::chrono::high_resolution_clock::now();
    lastTime = startTime;
    deltaTime = 0.0;
  }

  void update() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = currentTime - lastTime;
    deltaTime = elapsed.count();
    lastTime = currentTime;
  }

  double getDeltaTime() const { return deltaTime; }

  double getElapsedTime() const {
    std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - startTime;
    return elapsed.count();
  }

private:
  std::chrono::high_resolution_clock::time_point startTime;
  std::chrono::high_resolution_clock::time_point lastTime;
  double deltaTime;
};
} // namespace core
} // namespace engine