#pragma once

#include "types.h"

namespace Quasar
{

class LeaderRotation
{
  public:
	virtual Identity leader(Round round) = 0;
};

} // namespace Quasar
