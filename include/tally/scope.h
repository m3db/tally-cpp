#pragma once

#include <map>
#include <string>

#include "tally/capabilities.h"
#include "tally/counter.h"
#include "tally/gauge.h"
#include "tally/histogram.h"
#include "tally/timer.h"

namespace tally
{

class Scope
{
public:
  virtual std::shared_ptr<Counter> &Counter(const std::string &name);

  virtual std::shared_ptr<Gauge> &Gauge(const std::string &name);

  virtual std::shared_ptr<Timer> &Timer(const std::string &name);

  virtual std::shared_ptr<Histogram> &Histogram(const std::string &name);

  virtual std::unique_ptr<Scope> SubScope(const std::string &name);

  virtual std::unique_ptr<Scope> Tagged(const std::map<std::string, std::string> &tags, Buckets buckets);

  virtual std::unique_ptr<Capabilities> Capabilities();
};

} // namespace tally
