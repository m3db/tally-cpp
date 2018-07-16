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

#include <unordered_map>
#include <string>

#include "tally/buckets.h"
#include "tally/capabilities.h"
#include "tally/counter.h"
#include "tally/gauge.h"
#include "tally/histogram.h"
#include "tally/timer.h"

namespace tally {

class Scope {
 public:
  virtual ~Scope() = default;

  // Counter returns a new Counter with the provided name.
  virtual std::shared_ptr<tally::Counter> Counter(const std::string &name) = 0;

  // Gauge returns a new Gauge with the provided name.
  virtual std::shared_ptr<tally::Gauge> Gauge(const std::string &name) = 0;

  // Timer returns a new Timer with the provided name.
  virtual std::shared_ptr<tally::Timer> Timer(const std::string &name) = 0;

  // Histogram returns a new Histogram with the provided name.
  virtual std::shared_ptr<tally::Histogram> Histogram(
      const std::string &name, const Buckets &buckets) = 0;

  // SubScope creates a new child scope with the same tags as the parent but
  // with the additional name.
  virtual std::shared_ptr<Scope> SubScope(const std::string &name) = 0;

  // Tagged creates a new child scope with the same name as the parent and with
  // the tags of the parent and those provided. The provided tags take
  // precedence over the parent's tags.
  virtual std::shared_ptr<Scope> Tagged(
      const std::unordered_map<std::string, std::string> &tags) = 0;

  // Capabilities returns the Capabilities of the Scope.
  virtual std::unique_ptr<tally::Capabilities> Capabilities() = 0;
};

}  // namespace tally
