#pragma once

#include <chrono>
#include <vector>

#include "tally/stopwatch.h"

namespace tally
{

class Buckets
{
  using Clock = std::chrono::steady_clock;

public:
  virtual std::vector<double> AsValues();

  virtual std::vector<Clock::duration> AsDurations();
};

class Histogram
{
  using Clock = std::chrono::steady_clock;

public:
  virtual void RecordValue(double);
  virtual void RecordDuration(Clock::duration);
  virtual Stopwatch Start();
};

} // namespace tally
