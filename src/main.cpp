#include "core/application.hpp"
#include <core/logger.hpp>

int main() { return Application{1024, 768, "Simple Engine"}.run(); }