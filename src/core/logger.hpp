#pragma once

#include <chrono>
#include <format>
#include <iostream>
#include <sstream>
#include <string>

enum class LogLevel { Trace, Debug, Info, Warn, Error, Fatal };

class Logger {
public:
  template <LogLevel Level, typename... Args> static void log(std::format_string<Args...> fmt, Args &&...args) {
    std::cout << "[" << currentTime() << "]";
    std::cout << "[" << logLevelToString(Level) << "] ";
    std::cout << std::format(fmt, std::forward<Args>(args)...) << std::endl;
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

#define LOG_TRACE(fmt, ...) Logger::log<LogLevel::Trace>(fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) Logger::log<LogLevel::Debug>(fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) Logger::log<LogLevel::Info>(fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) Logger::log<LogLevel::Warn>(fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) Logger::log<LogLevel::Error>(fmt, ##__VA_ARGS__)
#define LOG_FATAL(fmt, ...) Logger::log<LogLevel::Fatal>(fmt, ##__VA_ARGS__)
