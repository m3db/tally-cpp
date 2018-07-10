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
#include <thread>

#include "gtest/gtest.h"

#include "mock_stats_reporter.h"
#include "tally/buckets.h"
#include "tally/scope_builder.h"
#include "tally/src/scope_impl.h"

class MockCapabilites : public tally::Capabilities {
 public:
  MOCK_METHOD0(Reporting, bool());
  MOCK_METHOD0(Tagging, bool());
};

TEST(ScopeImplTest, GetOrCreateCounter) {
  auto scope = tally::ScopeBuilder().Build();
  auto counter = scope->Counter("foo");
  EXPECT_EQ(counter, scope->Counter("foo"));
  EXPECT_NE(counter, scope->Counter("bar"));
}

TEST(ScopeImplTest, GetOrCreateGauge) {
  auto scope = tally::ScopeBuilder().Build();
  auto gauge = scope->Gauge("foo");
  EXPECT_EQ(gauge, scope->Gauge("foo"));
  EXPECT_NE(gauge, scope->Gauge("bar"));
}

TEST(ScopeImplTest, GetOrCreateTimer) {
  auto scope = tally::ScopeBuilder().Build();
  auto timer = scope->Timer("foo");
  EXPECT_EQ(timer, scope->Timer("foo"));
  EXPECT_NE(timer, scope->Timer("bar"));
}

TEST(ScopeImplTest, GetOrCreateHistogram) {
  auto scope = tally::ScopeBuilder().Build();
  auto buckets = tally::Buckets::LinearValues(1.0, 2.0, 6);
  auto histogram = scope->Histogram("foo", buckets);
  EXPECT_EQ(histogram, scope->Histogram("foo", buckets));
  EXPECT_NE(histogram, scope->Histogram("bar", buckets));
}

TEST(ScopeImplTest, GetOrCreateSubScope) {
  auto scope = tally::ScopeBuilder().Build();
  auto sub_scope = scope->SubScope("foo");
  EXPECT_EQ(sub_scope, scope->SubScope("foo"));
  EXPECT_NE(sub_scope, scope->SubScope("bar"));
}

TEST(ScopeImplTest, GetOrCreateTagged) {
  auto scope = tally::ScopeBuilder().Build();
  auto sub_scope = scope->Tagged({{"a", "1"}});
  EXPECT_EQ(sub_scope, scope->Tagged({{"a", "1"}}));
  EXPECT_NE(sub_scope, scope->Tagged({{"b", "2"}}));
}

TEST(ScopeImplTest, ValidReporterCapabilities) {
  auto capabilities = new MockCapabilites();
  auto reporter = std::make_shared<MockStatsReporter>();
  EXPECT_CALL(*reporter, CapabilitiesProxy())
      .WillOnce(testing::Return(capabilities));
  EXPECT_CALL(*capabilities, Reporting()).WillOnce(testing::Return(true));
  EXPECT_CALL(*capabilities, Tagging()).WillOnce(testing::Return(true));

  auto builder = tally::ScopeBuilder().reporter(reporter);
  auto scope = builder.Build();
  auto actual_capabilities = scope->Capabilities();
  EXPECT_EQ(true, actual_capabilities->Reporting());
  EXPECT_EQ(true, actual_capabilities->Tagging());
}

TEST(ScopeImplTest, NoReporterCapabilities) {
  auto scope = tally::ScopeBuilder().Build();
  auto capabilities = scope->Capabilities();
  EXPECT_EQ(false, capabilities->Reporting());
  EXPECT_EQ(false, capabilities->Tagging());
}

TEST(ScopeImplTest, ReportEvery) {
  std::string prefix("foo");
  std::string subscope_name("bar");
  std::string name("baz");
  std::map<std::string, std::string> scope_tags({{"a", "1"}});
  std::map<std::string, std::string> subscope_tags({{"b", "2"}});

  std::string expected_name = "foo.bar.baz";
  std::map<std::string, std::string> expected_tags = {{"a", "1"}, {"b", "2"}};
  auto buckets = tally::Buckets::LinearValues(0.0, 1.0, 10);

  auto reporter = std::make_shared<MockStatsReporter>();

  EXPECT_CALL(*reporter.get(), ReportCounter(expected_name, expected_tags, 1))
      .Times(1);
  EXPECT_CALL(*reporter.get(), ReportGauge(expected_name, expected_tags, 1.0))
      .Times(1);
  EXPECT_CALL(*reporter.get(), ReportTimer(expected_name, expected_tags,
                                           std::chrono::nanoseconds(1)))
      .Times(1);
  EXPECT_CALL(*reporter.get(),
              ReportHistogramValueSamples(expected_name, expected_tags, 3, 10,
                                          2.0, 3.0, 1))
      .Times(1);
  EXPECT_CALL(*reporter.get(), Flush()).Times(testing::AtLeast(1));

  auto scope = tally::ScopeBuilder()
                   .prefix(prefix)
                   .tags(scope_tags)
                   .reporter(reporter)
                   .ReportEvery(std::chrono::seconds(1));
  auto subscope = scope->SubScope(subscope_name)->Tagged(subscope_tags);

  auto counter = subscope->Counter(name);
  auto gauge = subscope->Gauge(name);
  auto timer = subscope->Timer(name);
  auto histogram = subscope->Histogram(name, buckets);

  counter->Inc();
  gauge->Update(1.0);
  timer->Record(std::chrono::nanoseconds(1));
  histogram->Record(2.5);

  // Wait for background thread to emit metrics.
  std::this_thread::sleep_for(std::chrono::seconds(3));
}
