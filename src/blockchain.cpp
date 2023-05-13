#include "blockchain.h"

#include "crypto.h"
#include <utility>

namespace Quasar
{

Blockchain::Blockchain()
{
	auto ptr = std::make_shared<Block>(GENESIS);
	m_committed = ptr;
	m_blocks.insert({ptr->hash(), ptr});
}

std::shared_ptr<Block> Blockchain::find(Hash hash) const
{
	auto entry = m_blocks.find(hash);
	if (entry == m_blocks.end())
	{
		return nullptr;
	}
	return entry->second;
}

void Blockchain::add(const Block &block)
{
	auto ptr = std::make_shared<Block>(block);
	auto hash = ptr->hash();
	m_blocks.insert({hash, ptr});
}

std::shared_ptr<Block> Blockchain::committed_block() const
{
	return m_committed;
}

void Blockchain::commit(const std::shared_ptr<Block> &block)
{
	auto parent = find(block->parent());
	if (m_committed->round() < parent->round())
	{
		commit(parent);
	}

	if (m_commit_handler)
	{
		m_commit_handler(block);
	}

	m_committed = block;
}

void Blockchain::set_commit_handler(std::function<void(std::shared_ptr<Block>)> handler)
{
	m_commit_handler = std::move(handler);
}

} // namespace Quasar