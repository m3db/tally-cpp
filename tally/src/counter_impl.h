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

#include <atomic>
#include <string>
#include <unordered_map>

#include "tally/counter.h"
#include "tally/stats_reporter.h"

namespace tally {

class CounterImpl : public Counter {
 public:
  CounterImpl();

  // Ensure the class is non-copyable.
  CounterImpl(const CounterImpl &) = delete;

  CounterImpl &operator=(const CounterImpl &) = delete;

  // Methods to implement the Counter interface.
  void Inc();

  void Inc(std::int64_t);

  // Report reports the current value of the counter.
  void Report(const std::string &name,
              const std::unordered_map<std::string, std::string> &tags,
              StatsReporter *reporter);

  // Value returns the current value of the counter.
  std::int64_t Value();

 private:
  std::atomic<std::int64_t> current_;
  std::atomic<std::int64_t> previous_;
};

}  // namespace tally
