#include <application/application.hpp>
#include <engine/core/logger.hpp>

int main()
{
  engine::core::Logger::init();
  Application{ 1024, 768, "Simple Engine" }.run();
  SDL_Quit();
  return EXIT_SUCCESS;
}
