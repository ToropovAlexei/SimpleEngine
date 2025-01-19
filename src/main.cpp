#include <application/application.hpp>
#include <cstdlib>
#include <engine/core/logger.hpp>

int main() {
  Application{1024, 768, "Simple Engine"}.run();
  SDL_Quit();
  return EXIT_SUCCESS;
}