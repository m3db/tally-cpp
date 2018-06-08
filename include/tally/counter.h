#pragma once

#include <cstdint>

namespace tally
{

class Counter
{
public:
  virtual void Inc(std::int64_t);
};

} // namespace tally
