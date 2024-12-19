#pragma once

#include <chrono>
#include <format>
#include <iostream>
#include <sstream>
#include <string>

enum class LogLevel { Trace, Debug, Info, Warn, Error, Fatal };

class Logger {
public:
  template <typename... Args> static void log(LogLevel level, const std::string &formatStr, Args &&...args) {
    std::string message = std::format(formatStr, std::forward<Args>(args)...);
    std::cout << "[" << currentDateTime() << "] ";
    std::cout << "[" << logLevelToString(level) << "] ";
    std::cout << message << std::endl;
  }

private:
  static std::string logLevelToString(LogLevel level) {
    switch (level) {
    case LogLevel::Trace:
      return "TRACE";
    case LogLevel::Debug:
      return "DEBUG";
    case LogLevel::Info:
      return "INFO";
    case LogLevel::Warn:
      return "WARN";
    case LogLevel::Error:
      return "ERROR";
    case LogLevel::Fatal:
      return "FATAL";
    default:
      return "UNKNOWN";
    }
  }

  static std::string currentDateTime() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&time);

    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
  }
};

// Макросы для удобства логирования
#define LOG_TRACE(fmt, ...) Logger::log(LogLevel::Trace, fmt, __VA_ARGS__)
#define LOG_DEBUG(fmt, ...) Logger::log(LogLevel::Debug, fmt, __VA_ARGS__)
#define LOG_INFO(fmt, ...) Logger::log(LogLevel::Info, fmt, __VA_ARGS__)
#define LOG_WARN(fmt, ...) Logger::log(LogLevel::Warn, fmt, __VA_ARGS__)
#define LOG_ERROR(fmt, ...) Logger::log(LogLevel::Error, fmt, __VA_ARGS__)
#define LOG_FATAL(fmt, ...) Logger::log(LogLevel::Fatal, fmt, __VA_ARGS__)
