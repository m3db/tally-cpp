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

#include <condition_variable>
#include <unordered_map>
#include <mutex>
#include <string>
#include <thread>

#include "tally/scope.h"
#include "tally/src/counter_impl.h"
#include "tally/src/gauge_impl.h"
#include "tally/src/histogram_impl.h"
#include "tally/src/timer_impl.h"
#include "tally/stats_reporter.h"

namespace tally {

class ScopeImpl : public Scope {
 public:
  ScopeImpl(const std::string &prefix, const std::string &separator,
            const std::unordered_map<std::string, std::string> &tags,
            std::chrono::seconds interval,
            std::shared_ptr<StatsReporter> reporter);

  ~ScopeImpl();

  // Ensure the class is non-copyable.
  ScopeImpl(const ScopeImpl &) = delete;

  ScopeImpl &operator=(const ScopeImpl &) = delete;

  // Methods to implement the Scope interface.
  std::shared_ptr<tally::Counter> Counter(const std::string &name);

  std::shared_ptr<tally::Gauge> Gauge(const std::string &name);

  std::shared_ptr<tally::Timer> Timer(const std::string &name);

  std::shared_ptr<tally::Histogram> Histogram(const std::string &name,
                                              const Buckets &buckets);

  std::shared_ptr<tally::Scope> SubScope(const std::string &name);

  std::shared_ptr<tally::Scope> Tagged(
      const std::unordered_map<std::string, std::string> &tags);

  std::unique_ptr<tally::Capabilities> Capabilities();

 private:
  // SubScope constructs a subscope with the provided prefix and tags.
  std::shared_ptr<tally::Scope> SubScope(
      const std::string &prefix,
      const std::unordered_map<std::string, std::string> &tags);

  // FullyQualifiedName returns the fully qualified name of the provided name.
  std::string FullyQualifiedName(const std::string &name);

  // ScopeID constructs a unique ID for a scope.
  static std::string ScopeID(const std::string &prefix,
                             const std::unordered_map<std::string, std::string> &tags);

  // Run is the function used to report metrics from the Scope.
  void Run();

  // Report reports the Scope's metrics to its Reporter.
  void Report();

  const std::string prefix_;
  const std::string separator_;
  const std::unordered_map<std::string, std::string> tags_;
  const std::chrono::nanoseconds interval_;
  std::shared_ptr<StatsReporter> reporter_;

  std::thread thread_;
  std::condition_variable cv_;
  std::mutex running_mutex_;
  bool running_;

  std::mutex registry_mutex_;
  std::unordered_map<std::string, std::shared_ptr<ScopeImpl>> registry_;

  std::mutex counters_mutex_;
  std::unordered_map<std::string, std::shared_ptr<CounterImpl>> counters_;

  std::mutex gauges_mutex_;
  std::unordered_map<std::string, std::shared_ptr<GaugeImpl>> gauges_;

  std::mutex timers_mutex_;
  std::unordered_map<std::string, std::shared_ptr<TimerImpl>> timers_;

  std::mutex histograms_mutex_;
  std::unordered_map<std::string, std::shared_ptr<HistogramImpl>> histograms_;
};

}  // namespace tally
