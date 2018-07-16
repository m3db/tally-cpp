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
#include "tally/buckets.h"
#include "tally/src/histogram_impl.h"

TEST(HistogramImplTest, RecordValueOnce) {
  std::string name("foo");
  std::unordered_map<std::string, std::string> tags({});
  auto buckets = tally::Buckets::LinearValues(0.0, 1.0, 10);
  double value = 1.5;
  std::shared_ptr<MockStatsReporter> reporter(new MockStatsReporter());

  EXPECT_CALL(*reporter.get(),
              ReportHistogramValueSamples(name, tags, 2, 10, 1.0, 2.0, 1));

  auto histogram = tally::HistogramImpl::New(buckets);
  histogram->Record(value);
  histogram->Report(name, tags, reporter.get());
}

TEST(HistogramImplTest, RecordDurationOnce) {
  std::string name("foo");
  std::unordered_map<std::string, std::string> tags({});
  auto buckets = tally::Buckets::ExponentialDurations(
      std::chrono::nanoseconds(1000), 2, 10);
  std::chrono::nanoseconds duration(1500);
  std::shared_ptr<MockStatsReporter> reporter(new MockStatsReporter());

  EXPECT_CALL(*reporter.get(),
              ReportHistogramDurationSamples(
                  name, tags, 1, 10, std::chrono::nanoseconds(1000),
                  std::chrono::nanoseconds(2000), 1));

  auto histogram = tally::HistogramImpl::New(buckets);
  histogram->Record(duration);
  histogram->Report(name, tags, reporter.get());
}

TEST(HistogramImplTest, RecordValueMultipleTimes) {
  std::string name("foo");
  std::unordered_map<std::string, std::string> tags({});
  auto buckets = tally::Buckets::LinearValues(0.0, 1.0, 10);
  double value = 1.5;
  std::shared_ptr<MockStatsReporter> reporter(new MockStatsReporter());

  EXPECT_CALL(*reporter.get(),
              ReportHistogramValueSamples(name, tags, 2, 10, 1.0, 2.0, 2));

  auto histogram = tally::HistogramImpl::New(buckets);
  histogram->Record(value);
  histogram->Record(value);
  histogram->Report(name, tags, reporter.get());
}

TEST(HistogramImplTest, RecordDurationMultipleTimes) {
  std::string name("foo");
  std::unordered_map<std::string, std::string> tags({});
  auto buckets = tally::Buckets::ExponentialDurations(
      std::chrono::nanoseconds(1000), 2, 10);
  std::chrono::nanoseconds duration(3000);
  std::shared_ptr<MockStatsReporter> reporter(new MockStatsReporter());

  EXPECT_CALL(*reporter.get(),
              ReportHistogramDurationSamples(
                  name, tags, 2, 10, std::chrono::nanoseconds(2000),
                  std::chrono::nanoseconds(4000), 2));

  auto histogram = tally::HistogramImpl::New(buckets);
  histogram->Record(duration);
  histogram->Record(duration);
  histogram->Report(name, tags, reporter.get());
}

TEST(HistogramImplTest, RecordValueMultipleTimesWithMultipleBuckets) {
  std::string name("foo");
  std::unordered_map<std::string, std::string> tags({});
  auto buckets = tally::Buckets::LinearValues(0.0, 1.0, 10);
  double first_value = 0.5;
  double second_value = 2.5;
  std::shared_ptr<MockStatsReporter> reporter(new MockStatsReporter());

  EXPECT_CALL(*reporter.get(),
              ReportHistogramValueSamples(name, tags, 1, 10, 0.0, 1.0, 1));
  EXPECT_CALL(*reporter.get(),
              ReportHistogramValueSamples(name, tags, 3, 10, 2.0, 3.0, 2));

  auto histogram = tally::HistogramImpl::New(buckets);
  histogram->Record(first_value);
  histogram->Record(second_value);
  histogram->Record(second_value);
  histogram->Report(name, tags, reporter.get());
}

TEST(HistogramImplTest, RecordDurationMultipleTimesWithMultipleBuckets) {
  std::string name("foo");
  std::unordered_map<std::string, std::string> tags({});
  auto buckets = tally::Buckets::ExponentialDurations(
      std::chrono::nanoseconds(1000), 2, 10);
  std::chrono::nanoseconds first_duration(3000);
  std::chrono::nanoseconds second_duration(1500);
  std::shared_ptr<MockStatsReporter> reporter(new MockStatsReporter());

  EXPECT_CALL(*reporter.get(),
              ReportHistogramDurationSamples(
                  name, tags, 1, 10, std::chrono::nanoseconds(1000),
                  std::chrono::nanoseconds(2000), 2));
  EXPECT_CALL(*reporter.get(),
              ReportHistogramDurationSamples(
                  name, tags, 2, 10, std::chrono::nanoseconds(2000),
                  std::chrono::nanoseconds(4000), 1));

  auto histogram = tally::HistogramImpl::New(buckets);
  histogram->Record(first_duration);
  histogram->Record(second_duration);
  histogram->Record(second_duration);
  histogram->Report(name, tags, reporter.get());
}

TEST(HistogramImplTest, Stopwatch) {
  std::string name("foo");
  std::unordered_map<std::string, std::string> tags({});
  auto buckets = tally::Buckets::LinearDurations(
      std::chrono::nanoseconds(0), std::chrono::nanoseconds(1000000), 10);
  std::shared_ptr<MockStatsReporter> reporter(new MockStatsReporter());

  EXPECT_CALL(*reporter.get(),
              ReportHistogramDurationSamples(
                  name, tags, 1, 10, std::chrono::nanoseconds(0),
                  std::chrono::nanoseconds(1000000), 1));

  auto histogram = tally::HistogramImpl::New(buckets);
  auto stopwatch = histogram->Start();
  stopwatch.Stop();
  histogram->Report(name, tags, reporter.get());
}

TEST(HistogramImplTest, RecordStopwatch) {
  std::string name("foo");
  std::unordered_map<std::string, std::string> tags({});
  auto buckets = tally::Buckets::LinearDurations(
      std::chrono::nanoseconds(0), std::chrono::nanoseconds(1000000), 10);
  std::shared_ptr<MockStatsReporter> reporter(new MockStatsReporter());

  EXPECT_CALL(*reporter.get(),
              ReportHistogramDurationSamples(
                  name, tags, 1, 10, std::chrono::nanoseconds(0),
                  std::chrono::nanoseconds(1000000), 1));

  auto histogram = tally::HistogramImpl::New(buckets);
  histogram->RecordStopwatch(std::chrono::steady_clock::now());
  histogram->Report(name, tags, reporter.get());
}
