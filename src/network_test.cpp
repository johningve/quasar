#include <catch2/catch_test_macros.hpp>

#include "network.h"

TEST_CASE("ZMQNetwork can connect", "[network]")
{
	auto net1 = std::make_shared<Quasar::ZMQNetwork>(8001);
	auto net2 = std::make_shared<Quasar::ZMQNetwork>(8002);

	REQUIRE_NOTHROW(net2->connect_to(Quasar::Identity{"00"}, "localhost:8001"));
}

TEST_CASE("ZMQNetwork can send message", "[network]")
{
	using namespace std::chrono_literals;

	const Quasar::Identity id1{"01"};

	auto net1 = std::make_shared<Quasar::ZMQNetwork>(8001);
	auto net2 = std::make_shared<Quasar::ZMQNetwork>(8002);

	REQUIRE_NOTHROW(net2->connect_to(id1, "localhost:8001"));

	Quasar::Round want = 10;
	bool received = false;

	Quasar::Proto::Message the_msg{};
	auto data_ptr = the_msg.mutable_data();
	auto wish_ptr = data_ptr->mutable_wish();
	wish_ptr->set_round(want);

	net1->set_message_handler([&](auto msg) {
		REQUIRE(msg.data().wish().round() == want);
		received = true;
	});

	REQUIRE_NOTHROW(net2->send_message(id1, the_msg));

	net1->poll(100ms);

	REQUIRE(received);
}
