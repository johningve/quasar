#pragma once

#include <queue>

#include "../network.h"

namespace Quasar
{

class TestNetwork;

class TestNetworkNode : Network
{
  public:
	explicit TestNetworkNode(std::shared_ptr<TestNetwork> m_network);

	void send_message(const Identity &recipient, const Proto::Message &msg) override;
	void broadcast_message(const Proto::Message &msg) override;
	void set_message_handler(std::function<void(Proto::Message &)> handler) override;

	void add_message(Proto::Message &msg);
	void handle_message();

  private:
	std::shared_ptr<TestNetwork> m_network;
	std::function<void(Proto::Message &)> m_message_handler;
	std::queue<Proto::Message> m_message_queue;
};

class TestNetwork : public std::enable_shared_from_this<TestNetwork>
{
  public:
	std::shared_ptr<TestNetworkNode> create_node(const Identity &id);
	void send_message(const Identity &recipient, const Proto::Message &msg);
	void run_for(int ticks);
	void run_single();

  private:
	std::unordered_map<Identity, std::shared_ptr<TestNetworkNode>> m_nodes;
};

} // namespace Quasar
