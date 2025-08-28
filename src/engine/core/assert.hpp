#pragma once
#include <engine/core/logger.hpp>
#include <source_location>

#ifndef NDEBUG
#define SE_ASSERT(condition, ...)                                                                           \
  do {                                                                                                      \
    if (!(condition)) {                                                                                     \
      core::Logger::fatal("Assertion failed: {} in {} at {}:{}", #condition, __FILE__, __func__, __LINE__); \
      core::Logger::fatal(__VA_ARGS__);                                                                     \
      if (std::getenv("SE_BREAK_ON_ASSERT")) { std::abort(); }                                              \
      std::terminate();                                                                                     \
    }                                                                                                       \
  } while (false)
#else
#define SE_ASSERT(condition, ...) ((void)0)
#endif


namespace engine::core {
template<typename... Args> struct unreachable
{
  explicit unreachable(fmt::format_string<Args...> fmt,
    Args &&...args,
    const std::source_location &loc = std::source_location::current())
  {
#ifndef NDEBUG
    core::Logger::fatal("Unreachable code reached in {} at {}:{}", loc.function_name(), loc.file_name(), loc.line());
    core::Logger::fatal(fmt, std::forward<Args>(args)...);
    std::terminate();
#endif
  }
};
template<typename... Args> unreachable(fmt::format_string<Args...>, Args &&...) -> unreachable<Args...>;
}// namespace engine::core