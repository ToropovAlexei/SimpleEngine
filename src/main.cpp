#include <application/application.hpp>

int main() {
  Application{1024, 768, "Simple Engine"}.run();
  SDL_Quit();
  return EXIT_SUCCESS;
}