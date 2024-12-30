#include <core/logger.hpp>

#define SE_THROW_ERROR(msg)                                                                                            \
  do {                                                                                                                 \
    LOG_FATAL("Runtime error: {}", msg);                                                                               \
    LOG_FATAL("Exception: {} at {}:{}", __FILE__, __func__, __LINE__);                                                 \
    throw std::runtime_error(std::string("Runtime error: ") + msg);                                                    \
  } while (false)