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

#include <mutex>
#include <vector>

#include "m3/thrift/M3.h"

class MockHandler : virtual public tally::m3::thrift::M3If {
 public:
  MockHandler() {}

  void emitMetricBatch(const tally::m3::thrift::MetricBatch& batch) {
    std::lock_guard<std::mutex> lock(mutex_);
    batches_.push_back(batch);
  }

  bool empty() {
    std::lock_guard<std::mutex> lock(mutex_);
    return batches_.size() == 0;
  }

  tally::m3::thrift::MetricBatch getBatch() {
    std::lock_guard<std::mutex> lock(mutex_);
    auto batch = batches_[0];
    batches_.clear();
    return batch;
  }

 private:
  std::mutex mutex_;
  std::vector<tally::m3::thrift::MetricBatch> batches_;
};
