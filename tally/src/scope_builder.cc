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

#include "tally/scope_builder.h"

#include "tally/src/scope_impl.h"

namespace tally {

namespace {
const std::string DEFAULT_PREFIX = "";
const std::string DEFAULT_SEPARATOR = ".";
const std::unordered_map<std::string, std::string> DEFAULT_TAGS = {};
const std::shared_ptr<StatsReporter> DEFAULT_REPORTER =
    NoopStatsReporter::New();
}  // namespace

ScopeBuilder::ScopeBuilder()
    : prefix_(DEFAULT_PREFIX),
      separator_(DEFAULT_SEPARATOR),
      tags_(DEFAULT_TAGS),
      reporter_(DEFAULT_REPORTER) {}

ScopeBuilder &ScopeBuilder::reporter(std::shared_ptr<StatsReporter> reporter) {
  reporter_ = reporter;
  return *this;
}

ScopeBuilder &ScopeBuilder::prefix(const std::string &prefix) {
  prefix_ = prefix;
  return *this;
}

ScopeBuilder &ScopeBuilder::separator(const std::string &separator) {
  separator_ = separator;
  return *this;
}

ScopeBuilder &ScopeBuilder::tags(
    const std::unordered_map<std::string, std::string> &tags) {
  tags_ = tags;
  return *this;
}

std::unique_ptr<Scope> ScopeBuilder::Build() {
  return ReportEvery(std::chrono::seconds(0));
}

std::unique_ptr<Scope> ScopeBuilder::ReportEvery(
    std::chrono::seconds interval) {
  return std::unique_ptr<Scope>{new ScopeImpl(
      this->prefix_, this->separator_, this->tags_, interval, this->reporter_)};
}

}  // namespace tally
