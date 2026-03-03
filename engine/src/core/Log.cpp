#include "engine/core/Log.hpp"

#include <chrono>
#include <cstdio>

namespace engine {
namespace {

const char* toString(LogLevel lvl) {
  switch (lvl) {
  case LogLevel::Trace: return "TRACE";
  case LogLevel::Debug: return "DEBUG";
  case LogLevel::Info: return "INFO ";
  case LogLevel::Warn: return "WARN ";
  case LogLevel::Error: return "ERROR";
  }
  return "?????";
}

} // namespace

void Log::write(LogLevel level, std::string_view msg) {
  using Clock = std::chrono::system_clock;
  const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now().time_since_epoch()).count();
  std::fprintf(stderr, "[%lld][%s] %.*s\n",
               static_cast<long long>(ms),
               toString(level),
               static_cast<int>(msg.size()),
               msg.data());
}

} // namespace engine


