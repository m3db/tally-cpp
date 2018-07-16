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

#pragma once

#include <map>
#include <string>

#include "m3/reporter.h"

namespace tally {

namespace m3 {

class ReporterBuilder {
 public:
  ReporterBuilder();

  // Methods to set various options for a Reporter.
  ReporterBuilder &host(const std::string &host);

  ReporterBuilder &port(uint16_t port);

  ReporterBuilder &common_tags(
      const std::map<std::string, std::string> &common_tags);

  ReporterBuilder &max_queue_size(uint32_t size);

  ReporterBuilder &max_packet_size(uint16_t size);

  // Build constructs the Reporter.
  std::shared_ptr<Reporter> Build();

 private:
  std::string host_;
  uint16_t port_;
  std::map<std::string, std::string> common_tags_;
  uint32_t max_queue_size_;
  uint32_t max_packet_size_;
};

}  // namespace m3

}  // namespace tally
