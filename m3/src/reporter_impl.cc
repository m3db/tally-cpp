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

#include "m3/src/reporter_impl.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <sstream>

#include "boost/format.hpp"
#include "thrift/protocol/TCompactProtocol.h"

#include "m3/reporter.h"
#include "m3/src/calc_transport.h"
#include "m3/thrift/M3.h"
#include "tally/src/capable_of.h"

using apache::thrift::protocol::TCompactProtocol;
using apache::thrift::transport::TTransportException;

namespace m3 {

namespace {
std::string HISTOGRAM_BUCKET_NAME = "bucket";
std::string HISTOGRAM_BUCKET_ID_NAME = "bucketid";
}  // namespace

Reporter::Impl::Impl(
    const std::string &host, uint16_t port,
    const std::unordered_map<std::string, std::string> &common_tags,
    uint32_t max_queue_size, uint16_t max_packet_size)
    : common_tags_(ConvertTags(common_tags)),
      max_queue_size_(max_queue_size),
      // Reserve 20% of the packet size for encoding overhead.
      max_packet_size_((max_packet_size / 5) * 4),
      run_(true) {
  emission_batch_.__set_commonTags(common_tags_);

  transport_ = std::make_shared<TUDPTransport>(
      host, port, TUDPTransport::Kind::Client, max_packet_size);

  transport_->open();

  emission_client_ = std::unique_ptr<thrift::M3Client>(
      new thrift::M3Client(std::make_shared<TCompactProtocol>(transport_)));

  thread_ = std::thread(&Reporter::Impl::Run, this);
}

Reporter::Impl::~Impl() {
  // Signal the background thread to shut down.
  {
    std::lock_guard<std::mutex> lock(run_mutex_);
    run_ = false;
  }

  // Wait for the background thread to finish.
  bg_thread_cv_.notify_one();
  thread_.join();

  transport_->close();
}

std::unique_ptr<tally::Capabilities> Reporter::Impl::Capabilities() {
  return std::unique_ptr<tally::Capabilities>(new tally::CapableOf(true, true));
}

void Reporter::Impl::ReportCounter(
    const std::string &name,
    const std::unordered_map<std::string, std::string> &tags, int64_t value) {
  auto counter = CreateCounter(value);
  auto metric_tags = ConvertTags(tags);
  ReportMetric(name, metric_tags, counter);
}

void Reporter::Impl::ReportGauge(
    const std::string &name,
    const std::unordered_map<std::string, std::string> &tags, double value) {
  auto gauge = CreateGauge(value);
  auto metric_tags = ConvertTags(tags);
  ReportMetric(name, metric_tags, gauge);
}

void Reporter::Impl::ReportTimer(
    const std::string &name,
    const std::unordered_map<std::string, std::string> &tags,
    std::chrono::nanoseconds value) {
  auto timer = CreateTimer(value);
  auto metric_tags = ConvertTags(tags);
  ReportMetric(name, metric_tags, timer);
}

void Reporter::Impl::ReportHistogramValueSamples(
    const std::string &name,
    const std::unordered_map<std::string, std::string> &tags,
    uint64_t bucket_id, uint64_t num_buckets, double buckets_lower_bound,
    double buckets_upper_bound, uint64_t samples) {
  auto counter = CreateCounter(samples);
  auto metric_tags = ConvertTags(tags);

  // Add tag for bucket.
  std::stringstream bucket_stream;
  bucket_stream << boost::format("%s-%s") %
                       ValueBucketString(buckets_lower_bound) %
                       ValueBucketString(buckets_upper_bound);

  thrift::MetricTag bucket_tag;
  bucket_tag.__set_tagName(HISTOGRAM_BUCKET_NAME);
  bucket_tag.__set_tagValue(bucket_stream.str());
  metric_tags.insert(bucket_tag);

  // Add tag for bucket ID.
  thrift::MetricTag id_tag;
  id_tag.__set_tagName(HISTOGRAM_BUCKET_ID_NAME);
  id_tag.__set_tagValue(BucketID(bucket_id, num_buckets));
  metric_tags.insert(id_tag);

  ReportMetric(name, metric_tags, counter);
}

void Reporter::Impl::ReportHistogramDurationSamples(
    const std::string &name,
    const std::unordered_map<std::string, std::string> &tags,
    uint64_t bucket_id, uint64_t num_buckets,
    std::chrono::nanoseconds buckets_lower_bound,
    std::chrono::nanoseconds buckets_upper_bound, uint64_t samples) {
  auto counter = CreateCounter(samples);
  auto metric_tags = ConvertTags(tags);

  // Add tag for bucket.
  std::stringstream bucket_stream;
  bucket_stream << boost::format("%s-%s") %
                       DurationBucketString(buckets_lower_bound) %
                       DurationBucketString(buckets_upper_bound);

  thrift::MetricTag bucket_tag;
  bucket_tag.__set_tagName(HISTOGRAM_BUCKET_NAME);
  bucket_tag.__set_tagValue(bucket_stream.str());
  metric_tags.insert(bucket_tag);

  // Add tag for bucket ID.
  thrift::MetricTag id_tag;
  id_tag.__set_tagName(HISTOGRAM_BUCKET_ID_NAME);
  id_tag.__set_tagValue(BucketID(bucket_id, num_buckets));
  metric_tags.insert(id_tag);

  ReportMetric(name, metric_tags, counter);
}

void Reporter::Impl::ReportMetric(const std::string &name,
                                  const std::set<thrift::MetricTag> &tags,
                                  thrift::MetricValue value) {
  thrift::Metric metric;
  metric.__set_name(name);
  metric.__set_tags(tags);
  metric.__set_metricValue(value);

  auto const now = std::chrono::system_clock::now();
  auto const nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(
      now.time_since_epoch());
  metric.__set_timestamp(nanos.count());

  Enqueue(metric);
}

thrift::MetricValue Reporter::Impl::CreateCounter(int64_t value) {
  thrift::CountValue count_value;
  count_value.__set_i64Value(value);
  thrift::MetricValue metric_value;
  metric_value.__set_count(count_value);
  return metric_value;
}

thrift::MetricValue Reporter::Impl::CreateGauge(double value) {
  thrift::GaugeValue gauge_value;
  gauge_value.__set_dValue(value);
  thrift::MetricValue metric_value;
  metric_value.__set_gauge(gauge_value);
  return metric_value;
}

thrift::MetricValue Reporter::Impl::CreateTimer(
    std::chrono::nanoseconds value) {
  thrift::TimerValue timer_value;
  timer_value.__set_i64Value(value.count());
  thrift::MetricValue metric_value;
  metric_value.__set_timer(timer_value);
  return metric_value;
}

std::set<thrift::MetricTag> Reporter::Impl::ConvertTags(
    const std::unordered_map<std::string, std::string> &tags) {
  std::set<thrift::MetricTag> metric_tags;
  for (auto const &entry : tags) {
    thrift::MetricTag tag;
    tag.__set_tagName(entry.first);
    tag.__set_tagValue(entry.second);
    metric_tags.insert(tag);
  }
  return metric_tags;
}

std::string Reporter::Impl::ValueBucketString(double bucket_bound) {
  if (bucket_bound == std::numeric_limits<double>::max()) {
    return "infinity";
  }

  if (bucket_bound == std::numeric_limits<double>::min()) {
    return "-infinity";
  }

  std::stringstream stream;
  stream << boost::format("%.6f") % bucket_bound;
  return stream.str();
}

std::string Reporter::Impl::DurationBucketString(
    std::chrono::nanoseconds bucket_bound) {
  if (bucket_bound == std::chrono::nanoseconds(0)) {
    return "0";
  }

  if (bucket_bound == std::numeric_limits<std::chrono::nanoseconds>::max()) {
    return "infinity";
  }

  if (bucket_bound == std::numeric_limits<std::chrono::nanoseconds>::min()) {
    return "-infinity";
  }

  // The format of time durations mimics the String method of Go's
  // time.Duration. It is designed so that the least granular unit is used such
  // that the first digit is not 0, e.g. 100µs instead of 0.1ms.
  bool is_negative = bucket_bound.count() < 0;
  bucket_bound = std::chrono::nanoseconds(std::abs(bucket_bound.count()));

  std::stringstream stream;
  if (is_negative) {
    stream << "-";
  }

  auto const seconds =
      std::chrono::duration_cast<std::chrono::seconds>(bucket_bound);
  if (seconds.count() < 1) {
    // Durations less than one second format use a smaller unit (milli-, micro-,
    // or nanoseconds) to ensure that the leading digit is non-zero.
    if (std::chrono::duration_cast<std::chrono::milliseconds>(bucket_bound)
            .count() > 0) {
      stream << FormatDuration(bucket_bound, 6);
      stream << "ms";
    } else if (std::chrono::duration_cast<std::chrono::milliseconds>(
                   bucket_bound)
                   .count() > 0) {
      stream << FormatDuration(bucket_bound, 3);
      stream << "µs";
    } else {
      stream << bucket_bound.count();
      stream << "ns";
    }
  } else {
    auto const hours =
        std::chrono::duration_cast<std::chrono::hours>(bucket_bound);
    auto const minutes =
        std::chrono::duration_cast<std::chrono::minutes>(bucket_bound - hours);
    auto const nanos = bucket_bound - hours - minutes;

    if (hours.count() > 0) {
      stream << hours.count();
      stream << "h";
    }

    if (minutes.count() > 0) {
      stream << minutes.count();
      stream << "m";
    }

    stream << FormatDuration(nanos, 9);
    stream << "s";
  }

  return stream.str();
}

std::string Reporter::Impl::BucketID(uint64_t bucket_id, uint64_t num_buckets) {
  int len = std::to_string(bucket_id).size();

  // Ensure the ID is at least 4 characters long.
  len = std::max(len, 4);

  std::string fmt = "%0" + std::to_string(len) + "d";

  std::stringstream stream;
  stream << boost::format(fmt) % bucket_id;

  return stream.str();
}

std::string Reporter::Impl::FormatDuration(std::chrono::nanoseconds duration,
                                           int precision) {
  // Format the duration into the fraction of the given precision (i.e v /
  // (10^p)). Omit any trailing zeros and the decimal when there is no
  // fractional part.
  double numerator = static_cast<double>(duration.count());
  double divisor = std::pow(10, precision);
  std::string str = std::to_string(numerator / divisor);

  // Remove trailing zeros.
  while (str[str.size() - 1] == '0') {
    str.erase(str.size() - 1);
  }

  // Remove the decimal if it is the last character.
  if (str[str.size() - 1] == '.') {
    str.erase(str.size() - 1);
  }

  return str;
}

void Reporter::Impl::Run() {
  // The lifetime of calc_transport is this Run method which is single threaded
  // so we don't need to protect it with a mutex.
  std::shared_ptr<TCalcTransport> calc_transport(new TCalcTransport());

  std::unique_lock<std::mutex> run_lock(run_mutex_);
  while (true) {
    if (!run_) {
      // When the reporter is closed, this thread needs to drain the queue and
      // flush any buffered metrics before it exits.
      std::lock_guard<std::mutex> queue_lock(queue_mutex_);
      while (queue_.size() > 0) {
        auto metric = queue_.front();
        Process(metric, calc_transport);
        queue_.pop();
      }
      Flush();
      return;
    }

    // While the queue is not empty this thread will continue to dequeue metrics
    // without waiting on the condition variable.
    while (true) {
      std::unique_lock<std::mutex> queue_lock(queue_mutex_);
      if (queue_.size() == 0) {
        break;
      }
      auto metric = queue_.front();

      // Release the queue lock so other threads can enqueue metrics while this
      // thread processes its current batch of metrics.
      queue_lock.unlock();

      Process(metric, calc_transport);

      queue_lock.lock();
      queue_.pop();
    }

    // Wait on condition variable at the end of the loop in case any metrics
    // were enqueued before the Run thread was started.
    bg_thread_cv_.wait(run_lock);
  }
}

void Reporter::Impl::Process(thrift::Metric metric,
                             std::shared_ptr<TCalcTransport> calc_transport) {
  TCompactProtocol calc_protocol(calc_transport);
  metric.write(&calc_protocol);
  auto const size = calc_transport->size();

  if (size > max_packet_size_) {
    Flush();
    calc_transport->flush();

    thrift::MetricBatch batch;
    batch.__set_commonTags(common_tags_);
    batch.write(&calc_protocol);

    // The current metric is not included in the batch of metrics that has just
    // been flushed so we need to re-add it's size after the reset.
    metric.write(&calc_protocol);
  }

  std::lock_guard<std::mutex> lock(emission_mutex_);
  emission_batch_.metrics.push_back(metric);
}

void Reporter::Impl::Flush() {
  std::lock_guard<std::mutex> lock(emission_mutex_);

  if (emission_batch_.metrics.empty()) {
    return;
  }

  try {
    emission_client_->emitMetricBatch(emission_batch_);
  } catch (const TTransportException &e) {
    std::cerr << "Encountered error emitting M3 metric batch: " << e.what()
              << std::endl;
  }

  emission_batch_.metrics.clear();
}

void Reporter::Impl::Enqueue(thrift::Metric metric) {
  std::lock_guard<std::mutex> lock(run_mutex_);
  if (!run_) {
    // This should never occur as the Reporter is only closed when it's
    // destructor is called which occurs only when no references to it remain.
    std::cerr << "Enqueue should never be called when the M3 reporter is not "
                 "running"
              << std::endl;
    return;
  }

  if (queue_.size() == max_queue_size_) {
    std::cerr << "Failed to enqueue metric because queue is full" << std::endl;
    return;
  }

  queue_.push(metric);
  bg_thread_cv_.notify_one();
}

}  // namespace m3
