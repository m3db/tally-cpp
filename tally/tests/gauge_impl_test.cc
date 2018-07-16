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
#include "tally/src/gauge_impl.h"

TEST(GaugeImplTest, UpdateOnce) {
  std::string name("foo");
  std::unordered_map<std::string, std::string> tags({});
  auto reporter = std::make_shared<MockStatsReporter>();

  EXPECT_CALL(*reporter.get(), ReportGauge(name, tags, 1.5)).Times(1);

  tally::GaugeImpl gauge;
  gauge.Update(1.5);
  gauge.Report(name, tags, reporter.get());
}

TEST(GaugeImplTest, UpdateMultipleTimes) {
  std::string name("foo");
  std::unordered_map<std::string, std::string> tags({});
  auto reporter = std::make_shared<MockStatsReporter>();

  EXPECT_CALL(*reporter.get(), ReportGauge(name, tags, 2.25)).Times(1);

  tally::GaugeImpl gauge;
  gauge.Update(1.5);
  gauge.Update(2.25);
  gauge.Report(name, tags, reporter.get());
}

TEST(GaugeImplTest, ValueIsReset) {
  std::string name("foo");
  std::unordered_map<std::string, std::string> tags({});
  auto reporter = std::make_shared<MockStatsReporter>();
  EXPECT_CALL(*reporter.get(), ReportGauge(name, tags, 1.5)).Times(1);
  EXPECT_CALL(*reporter.get(), ReportGauge(name, tags, 2.25)).Times(1);

  tally::GaugeImpl gauge;
  gauge.Update(1.5);
  gauge.Report(name, tags, reporter.get());

  gauge.Update(2.25);
  gauge.Report(name, tags, reporter.get());
}
