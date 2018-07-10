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

#include <cmath>
#include <cstdint>

#include "tally/src/buckets_calculator.h"

namespace tally {

BucketsCalculator::BucketsCalculator(Growth growth, double start, double update)
    : growth_(growth), start_(start), update_(update) {}

double BucketsCalculator::Calculate(uint64_t index) const {
  if (growth_ == BucketsCalculator::Growth::Exponential) {
    return start_ * std::pow(update_, static_cast<double>(index));
  }

  return start_ + (update_ * static_cast<double>(index));
}

bool BucketsCalculator::operator==(BucketsCalculator other) const {
  return growth_ == other.growth_ && start_ == other.start_ &&
         update_ == other.update_;
}

}  // namespace tally
