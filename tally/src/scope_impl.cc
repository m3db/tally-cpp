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

#include "tally/src/scope_impl.h"

#include <algorithm>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "tally/scope_builder.h"
#include "tally/src/capable_of.h"
#include "tally/src/noop_stats_reporter.h"

namespace tally {

ScopeImpl::ScopeImpl(const std::string &prefix, const std::string &separator,
                     const std::map<std::string, std::string> &tags,
                     std::chrono::seconds interval,
                     std::shared_ptr<StatsReporter> reporter)
    : prefix_(prefix),
      separator_(separator),
      tags_(tags),
      interval_(interval),
      reporter_((reporter == nullptr) ? NoopStatsReporter::New() : reporter),
      running_(false) {
  if (interval > std::chrono::seconds(0)) {
    running_ = true;
    thread_ = std::thread(&ScopeImpl::Run, this);
  }
}

ScopeImpl::~ScopeImpl() {
  // Signal the background thread to shut down if it's running.
  {
    std::lock_guard<std::mutex> lock(running_mutex_);
    if (!running_) {
      return;
    }
    running_ = false;
  }

  // Wait for the background thread to finish.
  cv_.notify_one();
  thread_.join();

  // Ensure any buffered metrics are emitted.
  Report();
}

std::shared_ptr<tally::Counter> ScopeImpl::Counter(const std::string &name) {
  std::lock_guard<std::mutex> lock(this->counters_mutex_);

  auto entry = this->counters_.insert(
      std::pair<std::string, std::shared_ptr<CounterImpl>>(
          name, std::shared_ptr<CounterImpl>(new CounterImpl())));
  return entry.first->second;
}

std::shared_ptr<tally::Gauge> ScopeImpl::Gauge(const std::string &name) {
  std::lock_guard<std::mutex> lock(this->gauges_mutex_);

  auto entry =
      this->gauges_.insert(std::pair<std::string, std::shared_ptr<GaugeImpl>>(
          name, std::shared_ptr<GaugeImpl>(new GaugeImpl())));
  return entry.first->second;
}

std::shared_ptr<tally::Timer> ScopeImpl::Timer(const std::string &name) {
  std::lock_guard<std::mutex> lock(this->timers_mutex_);

  // Since the timer reports metrics itself it must be initialized with the
  // fully qualified name.
  auto entry =
      this->timers_.insert(std::pair<std::string, std::shared_ptr<TimerImpl>>(
          name, TimerImpl::New(FullyQualifiedName(name), tags_, reporter_)));
  return entry.first->second;
}

std::shared_ptr<tally::Histogram> ScopeImpl::Histogram(const std::string &name,
                                                       const Buckets &buckets) {
  std::lock_guard<std::mutex> lock(this->histograms_mutex_);

  auto entry = this->histograms_.insert(
      std::pair<std::string, std::shared_ptr<HistogramImpl>>(
          name, HistogramImpl::New(buckets)));
  return entry.first->second;
}

std::shared_ptr<tally::Scope> ScopeImpl::SubScope(const std::string &name) {
  return SubScope(FullyQualifiedName(name), {});
}

std::shared_ptr<tally::Scope> ScopeImpl::Tagged(
    const std::map<std::string, std::string> &tags) {
  return SubScope(prefix_, tags);
}

std::unique_ptr<tally::Capabilities> ScopeImpl::Capabilities() {
  if (reporter_ == nullptr) {
    return std::unique_ptr<tally::Capabilities>(new CapableOf(false, false));
  }
  return reporter_->Capabilities();
}

std::shared_ptr<tally::Scope> ScopeImpl::SubScope(
    const std::string &prefix, const std::map<std::string, std::string> &tags) {
  std::map<std::string, std::string> new_tags;

  // Insert the new tags second as they take priority over the scope's tags.
  for (auto const &tag : tags_) {
    new_tags.insert(tag);
  }

  for (auto const &tag : tags) {
    new_tags.insert(tag);
  }

  auto id = ScopeID(prefix, new_tags);

  std::shared_ptr<Scope> scope = ScopeBuilder()
                                     .reporter(reporter_)
                                     .prefix(prefix)
                                     .separator(separator_)
                                     .tags(new_tags)
                                     .Build();

  std::lock_guard<std::mutex> lock(this->registry_mutex_);
  auto entry =
      this->registry_.insert(std::pair<std::string, std::shared_ptr<ScopeImpl>>(
          id, std::shared_ptr<ScopeImpl>(
                  std::dynamic_pointer_cast<ScopeImpl>(scope))));
  return entry.first->second;
}

std::string ScopeImpl::FullyQualifiedName(const std::string &name) {
  if (prefix_ == "") {
    return name;
  }

  std::string str;
  str.reserve(prefix_.length() + separator_.length() + name.length());
  std::stringstream stream(str);
  stream << prefix_ << separator_ << name;

  return stream.str();
}

std::string ScopeImpl::ScopeID(const std::string &prefix,
                               const std::map<std::string, std::string> &tags) {
  std::vector<std::string> keys;
  keys.reserve(tags.size());
  for (auto const &tag : tags) {
    keys.push_back(tag.first);
  }

  std::sort(keys.begin(), keys.end());

  std::string str;
  str.reserve(prefix.length() + keys.size() * 20);
  std::stringstream stream(str);

  stream << prefix;
  stream << "+";

  for (auto it = keys.begin(); it < keys.end(); it++) {
    auto key = *it;
    stream << key << "=" << tags.at(key);

    if (it != keys.end() - 1) {
      stream << ",";
    }
  }

  return stream.str();
}

void ScopeImpl::Run() {
  while (true) {
    std::unique_lock<std::mutex> running_lock(running_mutex_);
    cv_.wait_for(running_lock, interval_);

    Report();
    reporter_->Flush();

    // Check if the run loop has been stopped after flushing metrics so we can
    // ensure no buffered metrics were lost.
    if (!running_) {
      return;
    }
  }
}

void ScopeImpl::Report() {
  {
    std::lock_guard<std::mutex> lock(this->counters_mutex_);
    for (auto const &entry : counters_) {
      auto name = entry.first;
      auto counter = entry.second;
      counter->Report(FullyQualifiedName(name), tags_, reporter_.get());
    }
  }

  {
    std::lock_guard<std::mutex> lock(this->gauges_mutex_);
    for (auto const &entry : gauges_) {
      auto name = entry.first;
      auto gauge = entry.second;
      gauge->Report(FullyQualifiedName(name), tags_, reporter_.get());
    }
  }

  {
    std::lock_guard<std::mutex> lock(this->histograms_mutex_);
    for (auto const &entry : histograms_) {
      auto name = entry.first;
      auto histogram = entry.second;
      histogram->Report(FullyQualifiedName(name), tags_, reporter_.get());
    }
  }

  {
    std::lock_guard<std::mutex> lock(this->registry_mutex_);
    for (auto const &entry : registry_) {
      entry.second->Report();
    }
  }
}

}  // namespace tally
