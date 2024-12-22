#pragma once

#include <chrono>

class Clock {
public:
  Clock() { reset(); }

  void reset() {
    startTime = std::chrono::high_resolution_clock::now();
    lastTime = startTime;
    deltaTime = 0.0f;
  }

  void update() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = currentTime - lastTime;
    deltaTime = elapsed.count();
    lastTime = currentTime;
  }

  float getDeltaTime() const { return deltaTime; }

  float getElapsedTime() const {
    std::chrono::duration<float> elapsed =
        std::chrono::high_resolution_clock::now() - startTime;
    return elapsed.count();
  }

private:
  std::chrono::high_resolution_clock::time_point startTime;
  std::chrono::high_resolution_clock::time_point lastTime;
  float deltaTime;
};