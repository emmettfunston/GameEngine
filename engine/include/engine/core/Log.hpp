#pragma once

#include <string_view>

namespace engine {

enum class LogLevel : unsigned char {
  Trace,
  Debug,
  Info,
  Warn,
  Error,
};

class Log {
public:
  static void write(LogLevel level, std::string_view msg);

  static void trace(std::string_view msg) { write(LogLevel::Trace, msg); }
  static void debug(std::string_view msg) { write(LogLevel::Debug, msg); }
  static void info(std::string_view msg) { write(LogLevel::Info, msg); }
  static void warn(std::string_view msg) { write(LogLevel::Warn, msg); }
  static void error(std::string_view msg) { write(LogLevel::Error, msg); }
};

} // namespace engine


