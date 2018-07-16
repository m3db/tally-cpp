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

#include "m3/reporter_builder.h"

namespace m3 {

namespace {
constexpr uint32_t DEFAULT_MAX_QUEUE_SIZE = 1024;
constexpr uint16_t DEFAULT_MAX_PACKET_SIZE = 1440;
constexpr uint16_t DEFAULT_PORT = 9052;
const std::string DEFAULT_HOST = "127.0.0.1";
const std::unordered_map<std::string, std::string> DEFAULT_COMMON_TAGS = {};
}  // namespace

ReporterBuilder::ReporterBuilder()
    : host_(DEFAULT_HOST),
      port_(DEFAULT_PORT),
      common_tags_(DEFAULT_COMMON_TAGS),
      max_queue_size_(DEFAULT_MAX_QUEUE_SIZE),
      max_packet_size_(DEFAULT_MAX_PACKET_SIZE) {}

ReporterBuilder &ReporterBuilder::host(const std::string &host) {
  host_ = host;
  return *this;
}

ReporterBuilder &ReporterBuilder::port(uint16_t port) {
  port_ = port;
  return *this;
}

ReporterBuilder &ReporterBuilder::common_tags(
    const std::unordered_map<std::string, std::string> &common_tags) {
  common_tags_ = common_tags;
  return *this;
}

ReporterBuilder &ReporterBuilder::max_queue_size(uint32_t size) {
  max_queue_size_ = size;
  return *this;
}

ReporterBuilder &ReporterBuilder::max_packet_size(uint16_t size) {
  max_packet_size_ = size;
  return *this;
}

std::shared_ptr<Reporter> ReporterBuilder::Build() {
  return std::shared_ptr<Reporter>(new Reporter(
      host_, port_, common_tags_, max_queue_size_, max_packet_size_));
}

}  // namespace m3
