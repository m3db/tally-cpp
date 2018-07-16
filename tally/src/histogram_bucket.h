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

#include <string>
#include <unordered_map>
#include <vector>

#include "tally/buckets.h"
#include "tally/src/counter_impl.h"

namespace tally {

class HistogramBucket {
 public:
  HistogramBucket(Buckets::Kind kind, uint64_t bucket_id, uint64_t num_buckets,
                  double lower_bound, double upper_bound);

  void Record();
  void Report(const std::string &name,
              const std::unordered_map<std::string, std::string> &tags,
              StatsReporter *reporter);

  double lower_bound() const;
  double upper_bound() const;

 private:
  const Buckets::Kind kind_;
  const uint64_t bucket_id_;
  const uint64_t num_buckets_;
  const double lower_bound_;
  const double upper_bound_;
  std::shared_ptr<CounterImpl> samples_;
};

}  // namespace tally
