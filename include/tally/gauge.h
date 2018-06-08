#pragma once

namespace tally
{

class Gauge
{
public:
  virtual void Update(double);
};

} // namespace tally
