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

#include <cstdint>
#include <string>

#include "tally/src/counter_impl.h"

namespace tally {

// cppcheck reports a false positive error that previous is not initialized.
//
// cppcheck-suppress uninitMemberVar
CounterImpl::CounterImpl() noexcept : previous_(0), current_(0) {}

void CounterImpl::Inc() noexcept { Inc(1); }

void CounterImpl::Inc(int64_t delta) noexcept { current_ += delta; }

void CounterImpl::Report(
    const std::string &name,
    const std::unordered_map<std::string, std::string> &tags,
    StatsReporter *reporter) {
  auto const val = Value();
  if (val != 0 && reporter != nullptr) {
    reporter->ReportCounter(name, tags, val);
  }
}

int64_t CounterImpl::Value() {
  const auto current = current_.load();
  const auto previous = previous_;
  previous_ = current;
  return current - previous;
}

}  // namespace tally
