#pragma once

namespace tally
{

class Capabilities
{
public:
  virtual bool Reporting();

  virtual bool Tagging();
};

} // namespace tally