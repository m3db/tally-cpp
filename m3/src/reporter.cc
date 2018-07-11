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

#include "m3/reporter.h"

#include <chrono>
#include <map>
#include <string>

#include "m3/src/reporter_impl.h"
#include "tally/src/capable_of.h"

namespace tally {

namespace m3 {

Reporter::Reporter(const std::string &host, uint16_t port,
                   const std::map<std::string, std::string> &common_tags,
                   uint32_t max_queue_size, uint16_t max_packet_size)
    : impl_(new Reporter::Impl(host, port, common_tags, max_queue_size,
                               max_packet_size)) {}

Reporter::~Reporter() = default;

std::unique_ptr<tally::Capabilities> Reporter::Capabilities() {
  return impl_->Capabilities();
}

void Reporter::Flush() { impl_->Flush(); }

void Reporter::ReportCounter(const std::string &name,
                             const std::map<std::string, std::string> &tags,
                             int64_t value) {
  impl_->ReportCounter(name, tags, value);
}

void Reporter::ReportGauge(const std::string &name,
                           const std::map<std::string, std::string> &tags,
                           double value) {
  impl_->ReportGauge(name, tags, value);
}

void Reporter::ReportTimer(const std::string &name,
                           const std::map<std::string, std::string> &tags,
                           std::chrono::nanoseconds value) {
  impl_->ReportTimer(name, tags, value);
}

void Reporter::ReportHistogramValueSamples(
    const std::string &name, const std::map<std::string, std::string> &tags,
    uint64_t bucket_id, uint64_t num_buckets, double buckets_lower_bound,
    double buckets_upper_bound, uint64_t samples) {
  impl_->ReportHistogramValueSamples(name, tags, bucket_id, num_buckets,
                                     buckets_lower_bound, buckets_upper_bound,
                                     samples);
}

void Reporter::ReportHistogramDurationSamples(
    const std::string &name, const std::map<std::string, std::string> &tags,
    uint64_t bucket_id, uint64_t num_buckets,
    std::chrono::nanoseconds buckets_lower_bound,
    std::chrono::nanoseconds buckets_upper_bound, uint64_t samples) {
  impl_->ReportHistogramDurationSamples(name, tags, bucket_id, num_buckets,
                                        buckets_lower_bound,
                                        buckets_upper_bound, samples);
}

}  // namespace m3

}  // namespace tally
