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

#include "tally/src/timer_impl.h"

namespace tally {

TimerImpl::TimerImpl(const std::string &name,
                     const std::unordered_map<std::string, std::string> &tags,
                     std::shared_ptr<StatsReporter> reporter) noexcept
    : name_(name), tags_(tags), reporter_(reporter) {}

std::shared_ptr<TimerImpl> TimerImpl::New(
    const std::string &name,
    const std::unordered_map<std::string, std::string> &tags,
    std::shared_ptr<StatsReporter> reporter) noexcept {
  return std::shared_ptr<TimerImpl>(new TimerImpl(name, tags, reporter));
}

void TimerImpl::Record(std::chrono::nanoseconds value) {
  Record(static_cast<int64_t>(value.count()));
}

void TimerImpl::Record(int64_t value) {
  if (reporter_ != nullptr) {
    reporter_->ReportTimer(name_, tags_, std::chrono::nanoseconds(value));
  }
}

Stopwatch TimerImpl::Start() {
  return Stopwatch(std::chrono::steady_clock::now(), shared_from_this());
}

void TimerImpl::RecordStopwatch(std::chrono::steady_clock::time_point start) {
  auto const duration = std::chrono::steady_clock::now() - start;
  Record(duration);
}

}  // namespace tally
