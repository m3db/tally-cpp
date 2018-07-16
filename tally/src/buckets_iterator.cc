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

#include "tally/buckets_iterator.h"

#include "tally/src/buckets_calculator.h"

namespace tally {

BucketsIterator::BucketsIterator(BucketsCalculator calculator, uint64_t index)
    : calculator_(calculator), index_(index) {}

BucketsIterator &BucketsIterator::operator++() {
  index_++;
  return *this;
}

BucketsIterator BucketsIterator::operator++(int) {
  // Note that the postfix ++ operator takes a dummy int parameter to
  // distinguish it from the prefix ++ operator.
  const auto old_value = *this;
  ++(*this);
  return old_value;
}

bool BucketsIterator::operator==(BucketsIterator other) const {
  return calculator_ == other.calculator_ && index_ == other.index_;
}

bool BucketsIterator::operator!=(BucketsIterator other) const {
  return !(*this == other);
}

double BucketsIterator::operator*() { return calculator_.Calculate(index_); }

}  // namespace tally
