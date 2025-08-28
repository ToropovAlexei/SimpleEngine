#pragma once

#include <memory>

#ifdef NDEBUG
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#else
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#endif

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace engine::core {
class Logger
{
public:
  static void init()
  {
    if (s_logger) { return; };
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_pattern("%^[%T] %n: %v%$");

    s_logger = std::make_shared<spdlog::logger>("SimpleEngine", console_sink);
    s_logger->set_level(spdlog::level::trace);
    spdlog::register_logger(s_logger);
  }


  static std::shared_ptr<spdlog::logger> &get() { return s_logger; }

private:
  static std::shared_ptr<spdlog::logger> s_logger;
};

}// namespace engine::core

// Client log macros
#define LOG_TRACE(...) SPDLOG_LOGGER_TRACE(engine::core::Logger::get(), __VA_ARGS__)
#define LOG_DEBUG(...) SPDLOG_LOGGER_DEBUG(engine::core::Logger::get(), __VA_ARGS__)
#define LOG_INFO(...) SPDLOG_LOGGER_INFO(engine::core::Logger::get(), __VA_ARGS__)
#define LOG_WARN(...) SPDLOG_LOGGER_WARN(engine::core::Logger::get(), __VA_ARGS__)
#define LOG_ERROR(...) SPDLOG_LOGGER_ERROR(engine::core::Logger::get(), __VA_ARGS__)
#define LOG_FATAL(...) SPDLOG_LOGGER_CRITICAL(engine::core::Logger::get(), __VA_ARGS__)
