#pragma once

#include <optional>
#include <queue>

#include "types.h"

namespace Quasar
{

class Mempool
{
  public:
	void add(const Transaction &transaction);
	std::optional<Transaction> next();

  private:
	std::queue<Transaction> m_tx_queue;
};

} // namespace Quasar
