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
#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>

#include "gtest/gtest.h"
#include "thrift/protocol/TCompactProtocol.h"
#include "thrift/server/TSimpleServer.h"
#include "thrift/transport/TBufferTransports.h"
#include "thrift/transport/TServerSocket.h"

#include "m3/reporter.h"
#include "m3/reporter_builder.h"
#include "m3/thrift/M3.h"
#include "m3/thrift/m3_types.h"
#include "m3/udp_transport.h"
#include "mock_handler.h"
#include "mock_server.h"

class ReporterTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    server_ = std::shared_ptr<MockServer>(new MockServer("127.0.0.1", 0));
    thread_ = std::thread([this]() { server_->serve(); });
    reporter_ = tally::m3::ReporterBuilder()
                    .host("127.0.0.1")
                    .port(server_->port())
                    .max_queue_size(1000)
                    .max_packet_size(1024)
                    .Build();
  }

  virtual void TearDown() {
    server_->stop();
    thread_.join();
  }

  std::shared_ptr<MockServer> server_;
  std::shared_ptr<tally::m3::Reporter> reporter_;
  std::thread thread_;
};

TEST_F(ReporterTest, ReportCounter) {
  std::string name("foo");
  std::map<std::string, std::string> tags({{"a", "1"}});
  int64_t value = 1;

  std::set<tally::m3::thrift::MetricTag> expected_tags;
  for (auto const &entry : tags) {
    tally::m3::thrift::MetricTag tag;
    tag.__set_tagName(entry.first);
    tag.__set_tagValue(entry.second);
    expected_tags.insert(tag);
  }

  reporter_->ReportCounter(name, tags, value);
  reporter_->Flush();

  while (true) {
    if (!server_->isEmpty()) {
      break;
    }
    reporter_->Flush();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  auto batch = server_->getBatch();
  auto metric = batch.metrics[0];
  EXPECT_EQ(name, metric.name);
  EXPECT_EQ(expected_tags, metric.tags);
  auto metric_value = metric.metricValue;
  EXPECT_TRUE(metric_value.__isset.count);
  EXPECT_EQ(value, metric_value.count.i64Value);
}

TEST_F(ReporterTest, ReportGauge) {
  std::string name("foo");
  std::map<std::string, std::string> tags({{"a", "1"}});
  double value = 1.0;

  std::set<tally::m3::thrift::MetricTag> expected_tags;
  for (auto const &entry : tags) {
    tally::m3::thrift::MetricTag tag;
    tag.__set_tagName(entry.first);
    tag.__set_tagValue(entry.second);
    expected_tags.insert(tag);
  }

  reporter_->ReportGauge(name, tags, value);
  reporter_->Flush();

  while (true) {
    if (!server_->isEmpty()) {
      break;
    }

    reporter_->Flush();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  auto batch = server_->getBatch();
  auto metric = batch.metrics[0];
  EXPECT_EQ(name, metric.name);
  EXPECT_EQ(expected_tags, metric.tags);
  auto metric_value = metric.metricValue;
  EXPECT_TRUE(metric_value.__isset.gauge);
  EXPECT_EQ(value, metric_value.gauge.dValue);
}

TEST_F(ReporterTest, ReportTimer) {
  std::string name("foo");
  std::map<std::string, std::string> tags({{"a", "1"}});
  std::chrono::nanoseconds value(1);

  std::set<tally::m3::thrift::MetricTag> expected_tags;
  for (auto const &entry : tags) {
    tally::m3::thrift::MetricTag tag;
    tag.__set_tagName(entry.first);
    tag.__set_tagValue(entry.second);
    expected_tags.insert(tag);
  }

  reporter_->ReportTimer(name, tags, value);
  reporter_->Flush();

  while (true) {
    if (!server_->isEmpty()) {
      break;
    }

    reporter_->Flush();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  auto batch = server_->getBatch();
  auto metric = batch.metrics[0];
  EXPECT_EQ(name, metric.name);
  EXPECT_EQ(expected_tags, metric.tags);
  auto metric_value = metric.metricValue;
  EXPECT_TRUE(metric_value.__isset.timer);
  EXPECT_EQ(value.count(), metric_value.timer.i64Value);
}

TEST_F(ReporterTest, ReportHistogramValueSamples) {
  std::string name("foo");
  std::map<std::string, std::string> tags({{"a", "1"}});
  uint64_t value = 1;

  std::set<tally::m3::thrift::MetricTag> expected_tags;
  for (auto const &entry : tags) {
    tally::m3::thrift::MetricTag tag;
    tag.__set_tagName(entry.first);
    tag.__set_tagValue(entry.second);
    expected_tags.insert(tag);
  }

  tally::m3::thrift::MetricTag bucket_tag;
  bucket_tag.__set_tagName("bucket");
  bucket_tag.__set_tagValue("2.000000-3.000000");
  expected_tags.insert(bucket_tag);

  tally::m3::thrift::MetricTag id_tag;
  id_tag.__set_tagName("bucketid");
  id_tag.__set_tagValue("0002");
  expected_tags.insert(id_tag);

  reporter_->ReportHistogramValueSamples(name, tags, 2, 10, 2.0, 3.0, value);
  reporter_->Flush();

  while (true) {
    if (!server_->isEmpty()) {
      break;
    }

    reporter_->Flush();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  auto batch = server_->getBatch();
  auto metric = batch.metrics[0];
  EXPECT_EQ(name, metric.name);
  EXPECT_EQ(expected_tags, metric.tags);
  auto metric_value = metric.metricValue;
  EXPECT_TRUE(metric_value.__isset.count);
  EXPECT_EQ(value, metric_value.count.i64Value);
}

TEST_F(ReporterTest, ReportHistogramDurationSamples) {
  std::string name("foo");
  std::map<std::string, std::string> tags({{"a", "1"}});
  uint64_t value = 1;

  std::set<tally::m3::thrift::MetricTag> expected_tags;
  for (auto const &entry : tags) {
    tally::m3::thrift::MetricTag tag;
    tag.__set_tagName(entry.first);
    tag.__set_tagValue(entry.second);
    expected_tags.insert(tag);
  }

  tally::m3::thrift::MetricTag bucket_tag;
  bucket_tag.__set_tagName("bucket");
  bucket_tag.__set_tagValue("2ms-3ms");
  expected_tags.insert(bucket_tag);

  tally::m3::thrift::MetricTag id_tag;
  id_tag.__set_tagName("bucketid");
  id_tag.__set_tagValue("0002");
  expected_tags.insert(id_tag);

  reporter_->ReportHistogramDurationSamples(
      name, tags, 2, 10, std::chrono::nanoseconds(2000000),
      std::chrono::nanoseconds(3000000), value);
  reporter_->Flush();

  while (true) {
    if (!server_->isEmpty()) {
      break;
    }

    reporter_->Flush();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  auto batch = server_->getBatch();
  auto metric = batch.metrics[0];
  EXPECT_EQ(name, metric.name);
  EXPECT_EQ(expected_tags, metric.tags);
  auto metric_value = metric.metricValue;
  EXPECT_TRUE(metric_value.__isset.count);
  EXPECT_EQ(value, metric_value.count.i64Value);
}
