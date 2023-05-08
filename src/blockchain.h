#pragma once

#include <memory>
#include <unordered_map>

#include "types.h"

namespace Quasar
{

class Blockchain
{
  public:
	Blockchain();
	std::shared_ptr<Block> find(Hash hash) const;
	void add(Block block);
	// committed_block returns the most recently committed block
	std::shared_ptr<Block> committed_block() const;
	void commit(std::shared_ptr<Block> block);

  private:
	std::unordered_map<Hash, std::shared_ptr<Block>> m_blocks;
	std::shared_ptr<Block> m_committed;
};

} // namespace Quasar
