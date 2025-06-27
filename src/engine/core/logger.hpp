#pragma once

#include <chrono>
#include <format>
#include <print>
#include <sstream>
#include <string>

namespace engine {

namespace core {

enum class LogLevel { Trace, Debug, Info, Warn, Error, Fatal };

class Logger {
public:
  template <LogLevel Level, typename... Args> static void log(std::format_string<Args...> fmt, Args &&...args) {
    std::println("[{}][{}] {}", currentTime(), logLevelToString(Level), std::format(fmt, std::forward<Args>(args)...));
  }

private:
  static constexpr std::string logLevelToString(LogLevel level) {
    switch (level) {
    case LogLevel::Trace:
      return "\033[37mTRACE\033[0m"; // Grey
    case LogLevel::Debug:
      return "\033[34mDEBUG\033[0m"; // Blue
    case LogLevel::Info:
      return "\033[32mINFO\033[0m"; // Green
    case LogLevel::Warn:
      return "\033[33mWARN\033[0m"; // Yellow
    case LogLevel::Error:
      return "\033[31mERROR\033[0m"; // Red
    case LogLevel::Fatal:
      return "\033[41m\033[37mFATAL\033[0m"; // Red on Grey
    default:
      return "\033[0mUNKNOWN\033[0m"; // White
    }
  }

  static std::string currentTime() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&time);

    std::stringstream ss;
    ss << std::put_time(&tm, "%H:%M:%S");
    return ss.str();
  }
};

#ifndef NDEBUG
#define LOG_TRACE(fmt, ...) engine::core::Logger::log<engine::core::LogLevel::Trace>(fmt __VA_OPT__(, ) __VA_ARGS__)
#define LOG_DEBUG(fmt, ...) engine::core::Logger::log<engine::core::LogLevel::Debug>(fmt __VA_OPT__(, ) __VA_ARGS__)
#define LOG_INFO(fmt, ...) engine::core::Logger::log<engine::core::LogLevel::Info>(fmt __VA_OPT__(, ) __VA_ARGS__)
#define LOG_WARN(fmt, ...) engine::core::Logger::log<engine::core::LogLevel::Warn>(fmt __VA_OPT__(, ) __VA_ARGS__)
#define LOG_ERROR(fmt, ...) engine::core::Logger::log<engine::core::LogLevel::Error>(fmt __VA_OPT__(, ) __VA_ARGS__)
#define LOG_FATAL(fmt, ...) engine::core::Logger::log<engine::core::LogLevel::Fatal>(fmt __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOG_TRACE(fmt, ...)
#define LOG_DEBUG(fmt, ...)
#define LOG_INFO(fmt, ...)
#define LOG_WARN(fmt, ...)
#define LOG_ERROR(fmt, ...)
#define LOG_FATAL(fmt, ...)
#endif

} // namespace core
} // namespace engine