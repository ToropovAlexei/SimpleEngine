#pragma once
#include <engine/core/logger.hpp>
#include <source_location>


namespace engine::core {
template<typename... Args> struct assertion
{
  assertion(bool condition,
    fmt::format_string<Args...> fmt,
    Args &&...args,
    const std::source_location &loc = std::source_location::current())
  {
#ifndef NDEBUG
    if (!condition) [[unlikely]] {
      Logger::fatal("core::assertion failed in {} at {}:{}", loc.file_name(), loc.function_name(), loc.line());
      Logger::fatal(fmt, std::forward<Args>(args)...);

      std::terminate();
    }
#endif
  }
};

template<typename... Args> assertion(bool, fmt::format_string<Args...>, Args &&...) -> assertion<Args...>;

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