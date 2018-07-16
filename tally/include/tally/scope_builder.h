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

#include "tally/buckets.h"
#include "tally/scope.h"
#include "tally/src/noop_stats_reporter.h"
#include "tally/stats_reporter.h"

namespace tally {

class ScopeBuilder {
 public:
  ScopeBuilder();

  // Methods to set various options for a Scope.
  ScopeBuilder &reporter(std::shared_ptr<StatsReporter> reporter);

  ScopeBuilder &prefix(const std::string &prefix);

  ScopeBuilder &separator(const std::string &separator);

  ScopeBuilder &tags(const std::map<std::string, std::string> &tags);

  // ReportEvery constructs a scope and begins reporting metrics every
  // `interval` seconds.
  std::unique_ptr<Scope> ReportEvery(std::chrono::seconds interval);

  // Build constructs a Scope but does not start reporting metrics. Clients
  // should use ReportEvery instead of Build.
  std::unique_ptr<Scope> Build();

 private:
  std::string prefix_;
  std::string separator_;
  std::map<std::string, std::string> tags_;
  std::shared_ptr<StatsReporter> reporter_;
};

}  // namespace tally
