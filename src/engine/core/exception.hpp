#pragma once
#include <engine/core/logger.hpp>
#include <source_location>

namespace engine::core {
template<typename... Args> struct panic
{
  explicit panic(fmt::format_string<Args...> fmt,
    Args &&...args,
    const std::source_location &loc = std::source_location::current())
  {
    std::string message = fmt::format(fmt, std::forward<Args>(args)...);
    Logger::fatal("PANIC: {}", message);
    Logger::fatal("Location: {}:{} in {}", loc.file_name(), loc.line(), loc.function_name());

    throw std::runtime_error(
      fmt::format("PANIC at {}:{} in {}: {}", loc.file_name(), loc.line(), loc.function_name(), message));
  }
};

template<typename... Args> panic(fmt::format_string<Args...>, Args &&...) -> panic<Args...>;

}// namespace engine::core
