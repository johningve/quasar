#pragma once

#include "types.h"

namespace Quasar
{

class LeaderRotation
{
  public:
	virtual Identity leader(std::vector<Identity> all_nodes, Round round) = 0;
};

class RoundRobinLeaderRotation : public LeaderRotation
{
  public:
	Identity leader(std::vector<Identity> all_nodes, Round round) override;
};

} // namespace Quasar
