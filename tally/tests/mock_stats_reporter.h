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

#include <map>
#include <string>

#include "gmock/gmock.h"

#include "tally/stats_reporter.h"

class MockStatsReporter : public tally::StatsReporter {
  using Clock = std::chrono::steady_clock;

 public:
  // Since Capabilities returns a unique_ptr, which is non-copyable, we need a
  // proxy method which can return a pointer to the necessary Capabilities
  // object.
  std::unique_ptr<tally::Capabilities> Capabilities() override {
    return std::unique_ptr<tally::Capabilities>(CapabilitiesProxy());
  }

  MOCK_METHOD0(CapabilitiesProxy, tally::Capabilities *());

  MOCK_METHOD0(Flush, void());

  MOCK_METHOD3(ReportCounter,
               void(const std::string &name,
                    const std::map<std::string, std::string> &tags,
                    int64_t value));

  MOCK_METHOD3(ReportGauge, void(const std::string &name,
                                 const std::map<std::string, std::string> &tags,
                                 double value));

  MOCK_METHOD3(ReportTimer, void(const std::string &name,
                                 const std::map<std::string, std::string> &tags,
                                 std::chrono::nanoseconds value));

  MOCK_METHOD7(ReportHistogramValueSamples,
               void(const std::string &name,
                    const std::map<std::string, std::string> &tags,
                    uint64_t bucket_id, uint64_t num_buckets,
                    double buckets_lower_bound, double buckets_upper_bound,
                    uint64_t samples));

  MOCK_METHOD7(ReportHistogramDurationSamples,
               void(const std::string &name,
                    const std::map<std::string, std::string> &tags,
                    uint64_t bucket_id, uint64_t num_buckets,
                    std::chrono::nanoseconds buckets_lower_bound,
                    std::chrono::nanoseconds buckets_upper_bound,
                    uint64_t samples));
};
