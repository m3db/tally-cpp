#pragma once

#include <chrono>

#include "tally/stopwatch.h"

namespace tally
{

class Timer
{
  using Clock = std::chrono::steady_clock;

public:
  virtual void Record(Clock::duration);

  virtual Stopwatch Start();
};

} // namespace tally
