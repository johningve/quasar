#include <catch2/catch_test_macros.hpp>

#include "test_network.h"

TEST_CASE("Test TestNetwork", "[testing]")
{
	auto network = std::make_shared<Quasar::TestNetwork>();

	auto n1 = network->create_node(Quasar::Identity::from_hex_string("00"));
	auto n2 = network->create_node(Quasar::Identity::from_hex_string("01"));

	bool handler_ran = false;

	n2->set_message_handler([&handler_ran](auto _) { handler_ran = true; });
	n1->send_message(Quasar::Identity::from_hex_string("01"), Quasar::Proto::Message{});

	network->run_single();

	REQUIRE(handler_ran);
}