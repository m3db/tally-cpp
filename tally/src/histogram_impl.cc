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

#include "tally/src/histogram_impl.h"

#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

namespace tally {

HistogramImpl::HistogramImpl(const Buckets &buckets) noexcept
    : buckets_(CreateBuckets(buckets)) {}

std::shared_ptr<HistogramImpl> HistogramImpl::New(
    const Buckets &buckets) noexcept {
  return std::shared_ptr<HistogramImpl>(new HistogramImpl(buckets));
}

std::vector<HistogramBucket> HistogramImpl::CreateBuckets(
    const Buckets &buckets) {
  std::vector<HistogramBucket> histogram_buckets;
  auto size = buckets.size();
  auto kind = buckets.kind();
  if (size == 0) {
    histogram_buckets.push_back(
        HistogramBucket(kind, 0, 1, std::numeric_limits<double>::min(),
                        std::numeric_limits<double>::max()));
  } else {
    histogram_buckets.reserve(size);

    auto lower_bound = std::numeric_limits<double>::min();
    for (auto it = buckets.begin(); it != buckets.end(); it++) {
      auto upper_bound = *it;
      auto index = uint64_t(std::distance(buckets.begin(), it));
      histogram_buckets.push_back(HistogramBucket(kind, index, buckets.size(),
                                                  lower_bound, upper_bound));
      lower_bound = upper_bound;
    }

    // Add a catch-all bucket for anything past the last bucket.
    histogram_buckets.push_back(
        HistogramBucket(kind, buckets.size(), buckets.size(), lower_bound,
                        std::numeric_limits<double>::max()));
  }

  return histogram_buckets;
}

void HistogramImpl::Record(double val) noexcept {
  // Find the first bucket who's upper bound is greater than val.
  auto it = std::upper_bound(buckets_.begin(), buckets_.end(), val,
                             [](double lhs, const HistogramBucket &rhs) {
                               return lhs < rhs.upper_bound();
                             });
  it->Record();
}

void HistogramImpl::Record(std::chrono::nanoseconds val) noexcept {
  Record(static_cast<double>(val.count()));
}

Stopwatch HistogramImpl::Start() noexcept {
  return Stopwatch(std::chrono::steady_clock::now(), shared_from_this());
}

void HistogramImpl::RecordStopwatch(
    std::chrono::steady_clock::time_point start) {
  auto duration = std::chrono::steady_clock::now() - start;
  Record(duration);
}

void HistogramImpl::Report(
    const std::string &name,
    const std::unordered_map<std::string, std::string> &tags,
    StatsReporter *reporter) {
  for (auto &bucket : buckets_) {
    bucket.Report(name, tags, reporter);
  }
}

}  // namespace tally
