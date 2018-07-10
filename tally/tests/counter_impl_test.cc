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

#include "mock_stats_reporter.h"
#include "tally/src/counter_impl.h"

TEST(CounterImplTest, IncrementOnce) {
  std::string name("foo");
  std::map<std::string, std::string> tags({});
  auto reporter = std::make_shared<MockStatsReporter>();

  EXPECT_CALL(*reporter.get(), ReportCounter(name, tags, 1)).Times(1);

  tally::CounterImpl counter;
  counter.Inc(1);
  counter.Report(name, tags, reporter.get());
}

TEST(CounterImplTest, IncrementMultipleTimes) {
  std::string name("foo");
  std::map<std::string, std::string> tags({});
  auto reporter = std::make_shared<MockStatsReporter>();

  EXPECT_CALL(*reporter.get(), ReportCounter(name, tags, 3)).Times(1);

  tally::CounterImpl counter;
  counter.Inc(1);
  counter.Inc(2);
  counter.Report(name, tags, reporter.get());
}

TEST(CounterImplTest, ValueIsReset) {
  std::string name("foo");
  std::map<std::string, std::string> tags({});
  auto reporter = std::make_shared<MockStatsReporter>();

  EXPECT_CALL(*reporter.get(), ReportCounter(name, tags, 1)).Times(1);
  EXPECT_CALL(*reporter.get(), ReportCounter(name, tags, 2)).Times(1);

  tally::CounterImpl counter;
  counter.Inc(1);
  counter.Report(name, tags, reporter.get());

  counter.Inc(2);
  counter.Report(name, tags, reporter.get());
}
