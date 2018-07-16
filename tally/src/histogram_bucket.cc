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

#include "tally/src/histogram_bucket.h"

#include <unordered_map>
#include <string>
#include <vector>

namespace tally {

HistogramBucket::HistogramBucket(Buckets::Kind kind, uint64_t bucket_id,
                                 uint64_t num_buckets, double lower_bound,
                                 double upper_bound)
    : kind_(kind),
      bucket_id_(bucket_id),
      num_buckets_(num_buckets),
      lower_bound_(lower_bound),
      upper_bound_(upper_bound),
      samples_(new CounterImpl()) {}

void HistogramBucket::Record() { samples_->Inc(1); }

void HistogramBucket::Report(const std::string &name,
                             const std::unordered_map<std::string, std::string> &tags,
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

double HistogramBucket::lower_bound() const { return lower_bound_; }

double HistogramBucket::upper_bound() const { return upper_bound_; }

}  // namespace tally
