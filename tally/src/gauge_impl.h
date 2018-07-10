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
#include <map>
#include <string>

#include "tally/gauge.h"
#include "tally/stats_reporter.h"

namespace tally {

class GaugeImpl : public Gauge {
 public:
  GaugeImpl();

  // Ensure the class is non-copyable.
  GaugeImpl(const GaugeImpl &) = delete;

  GaugeImpl &operator=(const GaugeImpl &) = delete;

  // Methods to implement the Gauge interface.
  void Update(double);

  // Report reports the current value of the Gauge.
  void Report(const std::string &name,
              const std::map<std::string, std::string> &tags,
              StatsReporter *reporter);

 private:
  std::atomic<double> current_;
  std::atomic_bool updated_;
};

}  // namespace tally
