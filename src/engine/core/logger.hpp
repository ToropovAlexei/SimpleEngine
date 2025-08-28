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

  template<typename... Args> static void trace(fmt::format_string<Args...> fmt, Args &&...args)
  {
    Logger::get()->trace(fmt, std::forward<Args>(args)...);
  }

  template<typename... Args> static void debug(fmt::format_string<Args...> fmt, Args &&...args)
  {
    Logger::get()->debug(fmt, std::forward<Args>(args)...);
  }

  template<typename... Args> static void info(fmt::format_string<Args...> fmt, Args &&...args)
  {
    Logger::get()->info(fmt, std::forward<Args>(args)...);
  }

  template<typename... Args> static void warn(fmt::format_string<Args...> fmt, Args &&...args)
  {
    Logger::get()->warn(fmt, std::forward<Args>(args)...);
  }

  template<typename... Args> static void error(fmt::format_string<Args...> fmt, Args &&...args)
  {
    Logger::get()->error(fmt, std::forward<Args>(args)...);
  }

  template<typename... Args> static void fatal(fmt::format_string<Args...> fmt, Args &&...args)
  {
    Logger::get()->critical(fmt, std::forward<Args>(args)...);
  }


  static std::shared_ptr<spdlog::logger> &get() { return s_logger; }

private:
  static std::shared_ptr<spdlog::logger> s_logger;
};


}// namespace engine::core
