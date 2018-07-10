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

#include <chrono>

#include "gtest/gtest.h"

#include "mock_stats_reporter.h"
#include "tally/src/timer_impl.h"

TEST(TimerImplTest, RecordOnce) {
  std::string name("foo");
  std::map<std::string, std::string> tags({});
  auto reporter = std::make_shared<MockStatsReporter>();
  std::chrono::nanoseconds duration(1);

  EXPECT_CALL(*reporter, ReportTimer(name, tags, duration)).Times(1);

  auto timer = tally::TimerImpl::New(name, tags, reporter);
  timer->Record(duration);
}

TEST(TimerImplTest, RecordMultipleTimes) {
  std::string name("foo");
  std::map<std::string, std::string> tags({});
  auto reporter = std::make_shared<MockStatsReporter>();
  std::chrono::nanoseconds first_duration(1);
  std::chrono::nanoseconds second_duration(2);

  EXPECT_CALL(*reporter, ReportTimer(name, tags, first_duration)).Times(1);
  EXPECT_CALL(*reporter, ReportTimer(name, tags, second_duration)).Times(1);

  auto timer = tally::TimerImpl::New(name, tags, reporter);
  timer->Record(first_duration);
  timer->Record(second_duration);
}

TEST(TimerImplTest, Stopwatch) {
  std::string name("foo");
  std::map<std::string, std::string> tags({});
  auto reporter = std::make_shared<MockStatsReporter>();

  EXPECT_CALL(*reporter, ReportTimer(name, tags, testing::_)).Times(1);

  auto timer = tally::TimerImpl::New(name, tags, reporter);
  auto stopwatch = timer->Start();
  stopwatch.Stop();
}

TEST(TimerImplTest, RecordStopwatch) {
  std::string name("foo");
  std::map<std::string, std::string> tags({});
  auto reporter = std::make_shared<MockStatsReporter>();

  EXPECT_CALL(*reporter, ReportTimer(name, tags, testing::_)).Times(1);

  auto timer = tally::TimerImpl::New(name, tags, reporter);
  timer->RecordStopwatch(std::chrono::steady_clock::now());
}
