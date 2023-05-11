#include "mempool.h"

namespace Quasar
{

void Mempool::add(const Transaction &transaction)
{
	m_tx_queue.push(transaction);
}

Transaction Mempool::next()
{
	auto tx = m_tx_queue.front();
	m_tx_queue.pop();
	return tx;
}

} // namespace Quasar