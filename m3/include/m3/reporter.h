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

#include <chrono>
#include <map>
#include <memory>
#include <string>

#include "tally/stats_reporter.h"

namespace tally {

namespace m3 {

class Reporter : public tally::StatsReporter {
 public:
  friend class ReporterBuilder;

  ~Reporter();

  // Ensure the class is non-copyable.
  Reporter(const Reporter &) = delete;

  Reporter &operator=(const Reporter &) = delete;

  // Methods to implement the StatsReporter interface.
  std::unique_ptr<tally::Capabilities> Capabilities() override;

  void Flush() override;

  void ReportCounter(const std::string &name,
                     const std::map<std::string, std::string> &tags,
                     int64_t value) override;

  void ReportGauge(const std::string &name,
                   const std::map<std::string, std::string> &tags,
                   double value) override;

  void ReportTimer(const std::string &name,
                   const std::map<std::string, std::string> &tags,
                   std::chrono::nanoseconds value) override;

  void ReportHistogramValueSamples(
      const std::string &name, const std::map<std::string, std::string> &tags,
      uint64_t bucket_id, uint64_t num_buckets, double buckets_lower_bound,
      double buckets_upper_bound, uint64_t samples) override;

  void ReportHistogramDurationSamples(
      const std::string &name, const std::map<std::string, std::string> &tags,
      uint64_t bucket_id, uint64_t num_buckets,
      std::chrono::nanoseconds buckets_lower_bound,
      std::chrono::nanoseconds buckets_upper_bound, uint64_t samples) override;

 private:
  Reporter(const std::string &host, uint16_t port,
           const std::map<std::string, std::string> &common_tags,
           uint32_t max_queue_size, uint16_t max_packet_size);

  class Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace m3

}  // namespace tally
