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
#include <vector>

#include "tally/buckets_iterator.h"
#include "tally/src/buckets_calculator.h"

namespace tally {

class Buckets {
 public:
  // LinearValues constructs a linear sequence of `num` Value buckets beginning
  // at `start` and incrementing by `width` each time.
  static Buckets LinearValues(double start, double width, uint64_t num);

  // ExponentialValues constructs an expoential sequence of `num` Value buckets
  // beginning at `start` and multiplying by `factor` each time.
  static Buckets ExponentialValues(double start, double factor, uint64_t num);

  // LinearDurations constructs a linear sequence of `num` Duration buckets
  // beginning at `start` and incrementing by `width` each time.
  static Buckets LinearDurations(std::chrono::nanoseconds start,
                                 std::chrono::nanoseconds width, uint64_t num);

  // ExponentialDurations constructs an expoential sequence of `num` Duration
  // buckets beginning at `start` and multiplying by `factor` each time.
  static Buckets ExponentialDurations(std::chrono::nanoseconds start,
                                      uint64_t factor, uint64_t num);

  // Kind is an enum representing the type of a sequence of buckets.
  enum class Kind {
    Values,
    Durations,
  };

  BucketsIterator begin() const;

  BucketsIterator end() const;

  uint64_t size() const { return num_; }

  Kind kind() const { return kind_; }

 private:
  Buckets(Kind kind, BucketsCalculator calculator, uint64_t num);

  const Kind kind_;
  const BucketsCalculator calculator_;
  const double num_;
};

}  // namespace tally
