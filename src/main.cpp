#include "core/logger.hpp"

int main() {
  LOG_WARN("Test: {}, {}", 1, 2);
  LOG_ERROR("aadsf");
  //   std::cout << Log3("Test: {} {}", 1, 2) << std::endl;
  return 0;
}