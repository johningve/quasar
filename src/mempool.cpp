#include "mempool.h"

namespace Quasar
{

void Mempool::add(const Transaction &transaction)
{
	m_tx_queue.push(transaction);
}

std::optional<Transaction> Mempool::next()
{
	if (m_tx_queue.empty())
	{
		return std::nullopt;
	}
	auto tx = m_tx_queue.front();
	m_tx_queue.pop();
	return tx;
}

} // namespace Quasar