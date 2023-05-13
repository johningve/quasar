#pragma once

#include <functional>
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
	void add(const Block &block);
	// committed_block returns the most recently committed block
	std::shared_ptr<Block> committed_block() const;
	void commit(const std::shared_ptr<Block> &block);
	void set_commit_handler(std::function<void(std::shared_ptr<Block>)> handler);

  private:
	std::unordered_map<Hash, std::shared_ptr<Block>> m_blocks;
	std::shared_ptr<Block> m_committed;
	std::function<void(std::shared_ptr<Block>)> m_commit_handler;
};

} // namespace Quasar
