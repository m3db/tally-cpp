// Copyright (c) 2018 Uber Technologies, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#pragma once

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

#include "tally/buckets.h"
#include "tally/counter.h"
#include "tally/histogram.h"
#include "tally/src/counter_impl.h"
#include "tally/src/histogram_bucket.h"
#include "tally/stats_reporter.h"

namespace tally {
class HistogramImpl : public Histogram,
                      public StopwatchRecorder,
                      public std::enable_shared_from_this<StopwatchRecorder> {
 public:
  // New is used in place of the default constructor to ensure that callers are
  // returned a shared pointer to a HistogramImpl object since the class
  // inherits from the std::enable_shared_from_this class.
  static std::shared_ptr<HistogramImpl> New(const Buckets &buckets);

  // Ensure the class is non-copyable.
  HistogramImpl(const HistogramImpl &) = delete;

  HistogramImpl &operator=(const HistogramImpl &) = delete;

  // Methods to implement the Histogram interface.
  void Record(double);

  void Record(std::chrono::nanoseconds);

  Stopwatch Start();

  // Methods to implement the StopwatchRecorder interface.
  void RecordStopwatch(std::chrono::steady_clock::time_point);

  // Report reports the current values of the Histogram's buckets.
  void Report(const std::string &name,
              const std::unordered_map<std::string, std::string> &tags,
              StatsReporter *reporter);

 private:
  explicit HistogramImpl(const Buckets &buckets);

  static std::vector<HistogramBucket> CreateBuckets(const Buckets &buckets);

  std::vector<HistogramBucket> buckets_;
};

}  // namespace tally
