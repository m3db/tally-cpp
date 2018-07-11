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
#include <map>
#include <string>

namespace tally {

HistogramImpl::HistogramImpl(const Buckets &buckets)
    : buckets_(CreateBuckets(buckets)) {}

std::shared_ptr<HistogramImpl> HistogramImpl::New(const Buckets &buckets) {
  return std::shared_ptr<HistogramImpl>(new HistogramImpl(buckets));
}

std::vector<HistogramImpl::Bucket> HistogramImpl::CreateBuckets(
    const Buckets &buckets) {
  std::vector<Bucket> histogram_buckets;
  auto size = buckets.size();
  auto kind = buckets.kind();
  if (size == 0) {
    histogram_buckets.push_back(Bucket(kind, 0, 1,
                                       std::numeric_limits<double>::min(),
                                       std::numeric_limits<double>::max()));
  } else {
    histogram_buckets.reserve(size);

    auto lower_bound = std::numeric_limits<double>::min();
    for (auto it = buckets.begin(); it != buckets.end(); it++) {
      auto upper_bound = *it;
      auto index = uint64_t(std::distance(buckets.begin(), it));
      histogram_buckets.push_back(
          Bucket(kind, index, buckets.size(), lower_bound, upper_bound));
      lower_bound = upper_bound;
    }

    // Add a catch-all bucket for anything past the last bucket.
    histogram_buckets.push_back(Bucket(kind, buckets.size(), buckets.size(),
                                       lower_bound,
                                       std::numeric_limits<double>::max()));
  }

  return histogram_buckets;
}

void HistogramImpl::Record(double val) {
  // Find the first bucket who's upper bound is greater than val.
  auto it = std::upper_bound(buckets_.begin(), buckets_.end(), val,
                             [](double lhs, const HistogramImpl::Bucket &rhs) {
                               return lhs < rhs.upper_bound();
                             });
  it->Record();
}

void HistogramImpl::Record(std::chrono::nanoseconds val) {
  Record(static_cast<double>(val.count()));
}

Stopwatch HistogramImpl::Start() {
  return Stopwatch(std::chrono::steady_clock::now(), shared_from_this());
}

void HistogramImpl::RecordStopwatch(
    std::chrono::steady_clock::time_point start) {
  auto duration = std::chrono::steady_clock::now() - start;
  Record(duration);
}

void HistogramImpl::Report(const std::string &name,
                           const std::map<std::string, std::string> &tags,
                           StatsReporter *reporter) {
  for (auto &bucket : buckets_) {
    bucket.Report(name, tags, reporter);
  }
}

HistogramImpl::Bucket::Bucket(Buckets::Kind kind, uint64_t bucket_id,
                              uint64_t num_buckets, double lower_bound,
                              double upper_bound)
    : kind_(kind),
      bucket_id_(bucket_id),
      num_buckets_(num_buckets),
      lower_bound_(lower_bound),
      upper_bound_(upper_bound),
      samples_(new CounterImpl()) {}

void HistogramImpl::Bucket::Record() { samples_->Inc(1); }

void HistogramImpl::Bucket::Report(
    const std::string &name, const std::map<std::string, std::string> &tags,
    StatsReporter *reporter) {
  auto samples = samples_->Value();
  if (samples != 0 && reporter != nullptr) {
    if (kind_ == Buckets::Kind::Values) {
      reporter->ReportHistogramValueSamples(name, tags, bucket_id_,
                                            num_buckets_, lower_bound_,
                                            upper_bound_, samples);
    } else {
      reporter->ReportHistogramDurationSamples(
          name, tags, bucket_id_, num_buckets_,
          std::chrono::nanoseconds(static_cast<int64_t>(lower_bound_)),
          std::chrono::nanoseconds(static_cast<int64_t>(upper_bound_)),
          samples);
    }
  }
}

double HistogramImpl::Bucket::lower_bound() const { return lower_bound_; }

double HistogramImpl::Bucket::upper_bound() const { return upper_bound_; }

}  // namespace tally
