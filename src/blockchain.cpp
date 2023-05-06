#include "blockchain.h"
#include "crypto.h"

namespace Quasar
{

Blockchain::Blockchain()
{
	auto ptr = std::make_shared<Block>(GENESIS);
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

void Blockchain::add(Block block)
{
	auto ptr = std::make_shared<Block>(block);
	auto hash = ptr->hash();
	m_blocks.insert({hash, ptr});
}

} // namespace Quasar