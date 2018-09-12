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

#include "gtest/gtest.h"

#include "tally/buckets.h"

TEST(BucketsTest, LinearIteratorBuckets) {
  double start = 4.0;
  double width = 6.0;
  uint64_t num = 3.0;
  auto buckets = tally::Buckets::LinearValues(start, width, num);

  double expected = start;
  for (auto it = buckets.begin(); it != buckets.end(); it++) {
    EXPECT_EQ(expected, *it);
    expected += width;
  }
}

TEST(BucketsTest, LinearIteratorEquality) {
  auto buckets = tally::Buckets::LinearValues(1.0, 2.0, 10);

  auto other_it = buckets.begin();
  for (auto it = buckets.begin(); it != buckets.end(); it++) {
    EXPECT_EQ(other_it, it);
    other_it++;
  }
}

TEST(BucketsTest, ExponentialIteratorBuckets) {
  double start = 4.0;
  double factor = 2.0;
  uint64_t num = 3.0;
  auto buckets = tally::Buckets::ExponentialValues(start, factor, num);

  double expected = start;
  for (auto it = buckets.begin(); it != buckets.end(); it++) {
    EXPECT_EQ(expected, *it);
    expected *= factor;
  }
}

TEST(BucketsTest, ExponentialIteratorEquality) {
  auto buckets = tally::Buckets::ExponentialValues(1.0, 2.0, 10);

  auto other_it = buckets.begin();
  for (auto it = buckets.begin(); it != buckets.end(); it++) {
    EXPECT_EQ(other_it, it);
    other_it++;
  }
}

TEST(BucketsTest, Size) {
  auto linear_buckets = tally::Buckets::ExponentialValues(1.0, 2.0, 4);
  EXPECT_EQ(4, linear_buckets.size());

  auto exponential_buckets = tally::Buckets::ExponentialValues(1.0, 2.0, 10);
  EXPECT_EQ(10, exponential_buckets.size());
}
