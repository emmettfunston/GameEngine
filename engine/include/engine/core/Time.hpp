#pragma once

#include <chrono>

namespace engine {

class Time {
public:
  using Clock = std::chrono::steady_clock;

  void reset();
  float tickSeconds(); // returns dt since last tick

private:
  Clock::time_point m_last = Clock::now();
};

} // namespace engine


