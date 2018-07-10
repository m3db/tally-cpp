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

#include "tally/buckets.h"

#include <stdexcept>

#include "tally/buckets_iterator.h"
#include "tally/src/buckets_calculator.h"

namespace tally {

Buckets::Buckets(Buckets::Kind kind, BucketsCalculator calculator, uint64_t num)
    : kind_(kind), calculator_(calculator), num_(num) {
  if (num == 0) {
    throw std::invalid_argument("Number of buckets cannot be zero");
  }
}

Buckets Buckets::LinearValues(double start, double width, uint64_t num) {
  if (width <= 0) {
    throw std::invalid_argument("Bucket width must be positive");
  }

  auto calculator =
      BucketsCalculator(BucketsCalculator::Growth::Linear, start, width);
  return Buckets(Buckets::Kind::Values, calculator, num);
}

Buckets Buckets::ExponentialValues(double start, double factor, uint64_t num) {
  if (factor <= 1) {
    throw std::invalid_argument("Bucket factor must be greater than 1");
  }

  auto calculator =
      BucketsCalculator(BucketsCalculator::Growth::Exponential, start, factor);
  return Buckets(Buckets::Kind::Values, calculator, num);
}

Buckets Buckets::LinearDurations(std::chrono::nanoseconds start,
                                 std::chrono::nanoseconds width, uint64_t num) {
  if (width <= std::chrono::nanoseconds(0)) {
    throw std::invalid_argument("Bucket width must be positive");
  }
  auto calculator = BucketsCalculator(BucketsCalculator::Growth::Linear,
                                      static_cast<double>(start.count()),
                                      static_cast<double>(width.count()));
  return Buckets(Buckets::Kind::Durations, calculator, num);
}

Buckets Buckets::ExponentialDurations(std::chrono::nanoseconds start,
                                      uint64_t factor, uint64_t num) {
  if (factor <= 1) {
    throw std::invalid_argument("Bucket factor must be greater than 1");
  }

  auto calculator = BucketsCalculator(BucketsCalculator::Growth::Exponential,
                                      static_cast<double>(start.count()),
                                      static_cast<double>(factor));
  return Buckets(Buckets::Kind::Durations, calculator, num);
}

BucketsIterator Buckets::begin() const {
  return BucketsIterator(calculator_, 0);
}

BucketsIterator Buckets::end() const {
  return BucketsIterator(calculator_, num_);
}

uint64_t Buckets::size() const { return num_; }

Buckets::Kind Buckets::kind() const { return kind_; }

}  // namespace tally
