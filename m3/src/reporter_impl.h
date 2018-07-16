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
#include <condition_variable>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <string>
#include <thread>

#include "thrift/protocol/TCompactProtocol.h"

#include "m3/reporter.h"
#include "m3/src/calc_transport.h"
#include "m3/thrift/M3.h"
#include "m3/thrift/m3_types.h"
#include "m3/udp_transport.h"
#include "tally/stats_reporter.h"

using apache::thrift::transport::TTransport;

namespace tally {

namespace m3 {

class Reporter::Impl : public tally::StatsReporter {
 public:
  Impl(const std::string &host, uint16_t port,
       const std::unordered_map<std::string, std::string> &common_tags,
       uint32_t max_queue_size, uint16_t max_packet_size);

  ~Impl();

  // Ensure the class is non-copyable.
  Impl(const Impl &) = delete;

  Impl &operator=(const Impl &) = delete;

  // Methods to implement the StatsReporter interface.
  std::unique_ptr<tally::Capabilities> Capabilities();

  void Flush();

  void ReportCounter(const std::string &name,
                     const std::unordered_map<std::string, std::string> &tags,
                     int64_t value);

  void ReportGauge(const std::string &name,
                   const std::unordered_map<std::string, std::string> &tags,
                   double value);

  void ReportTimer(const std::string &name,
                   const std::unordered_map<std::string, std::string> &tags,
                   std::chrono::nanoseconds value);

  void ReportHistogramValueSamples(
      const std::string &name, const std::unordered_map<std::string, std::string> &tags,
      uint64_t bucket_id, uint64_t num_buckets, double buckets_lower_bound,
      double buckets_upper_bound, uint64_t samples);

  void ReportHistogramDurationSamples(
      const std::string &name, const std::unordered_map<std::string, std::string> &tags,
      uint64_t bucket_id, uint64_t num_buckets,
      std::chrono::nanoseconds buckets_lower_bound,
      std::chrono::nanoseconds buckets_upper_bound, uint64_t samples);

 private:
  // Run implements the logic of the Reporter, pulling metrics of its queue and
  // emitting them.
  void Run();

  // Enqueue adds a metric to the Reporter's queue.
  void Enqueue(thrift::Metric);

  // Process processes a metric so it can be emitted.
  void Process(thrift::Metric metric,
               std::shared_ptr<TCalcTransport> calc_transport);

  // Helper methods used when processing metrics.
  void ReportMetric(const std::string &name,
                    const std::set<thrift::MetricTag> &tags,
                    thrift::MetricValue value);

  thrift::MetricValue CreateCounter(int64_t value);

  thrift::MetricValue CreateGauge(double value);

  thrift::MetricValue CreateTimer(std::chrono::nanoseconds value);

  std::set<thrift::MetricTag> ConvertTags(
      const std::unordered_map<std::string, std::string> &tags);

  std::string ValueBucketString(double bucket_bound);

  std::string DurationBucketString(std::chrono::nanoseconds bucket_bound);

  std::string BucketID(uint64_t bucket_id, uint64_t num_buckets);

  std::string FormatDuration(std::chrono::nanoseconds duration, int precision);

  const std::set<thrift::MetricTag> common_tags_;
  const uint32_t max_queue_size_;
  const uint16_t max_packet_size_;

  std::condition_variable bg_thread_cv_;
  std::thread thread_;
  std::shared_ptr<TUDPTransport> transport_;

  std::mutex run_mutex_;
  bool run_;

  std::mutex emission_mutex_;
  thrift::MetricBatch emission_batch_;
  std::unique_ptr<thrift::M3Client> emission_client_;

  std::mutex queue_mutex_;
  std::queue<thrift::Metric> queue_;
};

}  // namespace m3

}  // namespace tally
