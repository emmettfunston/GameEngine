#include "engine/core/Time.hpp"

namespace engine {

void Time::reset() {
  m_last = Clock::now();
}

float Time::tickSeconds() {
  const auto now = Clock::now();
  const std::chrono::duration<float> dt = now - m_last;
  m_last = now;
  return dt.count();
}

} // namespace engine


