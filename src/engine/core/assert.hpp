#include <engine/core/logger.hpp>

#ifndef NDEBUG
#define SE_ASSERT(condition, ...)                                                                                      \
  do {                                                                                                                 \
    if (!(condition)) {                                                                                                \
      LOG_FATAL("Assertion failed: {} in {} at {}:{}", #condition, __FILE__, __func__, __LINE__);                      \
      LOG_FATAL(__VA_ARGS__);                                                                                          \
      if (std::getenv("SE_BREAK_ON_ASSERT")) {                                                                         \
        std::abort();                                                                                                  \
      }                                                                                                                \
      std::terminate();                                                                                                \
    }                                                                                                                  \
  } while (false)
#else
#define SE_ASSERT(condition, ...) ((void)0)
#endif

#ifndef NDEBUG
#define SE_UNREACHABLE(...)                                                                                            \
  do {                                                                                                                 \
    LOG_FATAL("Unreachable code reached in {} at {}:{}", __func__, __FILE__, __LINE__);                                \
    LOG_FATAL(__VA_ARGS__);                                                                                            \
    if (std::getenv("SE_BREAK_ON_UNREACHABLE")) {                                                                      \
      std::abort();                                                                                                    \
    }                                                                                                                  \
    std::terminate();                                                                                                  \
  } while (false)
#else
#if defined(__GNUC__) || defined(__clang__)
#define SE_UNREACHABLE(...) __builtin_unreachable()
#elif defined(_MSC_VER)
#define SE_UNREACHABLE(...) __assume(0)
#else
#define SE_UNREACHABLE(...) std::terminate()
#endif
#endif