#include <catch2/catch_test_macros.hpp>
#include <sstream>

#include "blockchain.h"

TEST_CASE("Get Genesis", "[blockchain]")
{
	Quasar::Blockchain blockchain;

	auto block = blockchain.find(Quasar::GENESIS.hash());
	REQUIRE(block != nullptr);
	REQUIRE(block->hash() == Quasar::GENESIS.hash());
}
