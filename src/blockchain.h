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

  private:
	std::unordered_map<Hash, std::shared_ptr<Block>> m_blocks;
};

} // namespace Quasar
