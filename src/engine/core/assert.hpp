#include <engine/core/logger.hpp>

#ifndef NDEBUG
#define SE_ASSERT(condition, msg)                                                                                      \
  do {                                                                                                                 \
    if (!(condition)) {                                                                                                \
      LOG_FATAL(msg);                                                                                                  \
      LOG_FATAL("Assertion failed: {} in {} at {}:{}", #condition, __FILE__, __func__, __LINE__);                      \
      throw std::runtime_error("Assertion failed: " #condition);                                                       \
    }                                                                                                                  \
  } while (false)
#else
#define SE_ASSERT(condition) (void)0
#endif
