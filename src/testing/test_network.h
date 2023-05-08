#pragma once

#include <queue>

#include "../network.h"

namespace Quasar
{

class TestNetwork;

class TestNetworkNode : public Network
{
  public:
	explicit TestNetworkNode(Identity identity, std::shared_ptr<TestNetwork> m_network);

  public:
	void send_message(const Identity &recipient, const Proto::Message &msg) override;
	void broadcast_message(const Proto::Message &msg) override;
	void set_message_handler(std::function<void(Proto::Message &)> handler) override;
	int size() override;

	void add_message(const Proto::Message &msg);
	void handle_message();

  private:
	Identity m_id;
	std::shared_ptr<TestNetwork> m_network;
	std::function<void(Proto::Message &)> m_message_handler;
	std::queue<Proto::Message> m_message_queue;
};

class TestNetwork : public std::enable_shared_from_this<TestNetwork>
{
  public:
	std::shared_ptr<TestNetworkNode> create_node(const Identity &id);
	void send_message(const Identity &recipient, const Proto::Message &msg);
	void broadcast_message(const Identity &id_from, const Proto::Message &msg);
	int size();
	void run_for(int ticks);
	void run_single();

  private:
	std::unordered_map<Identity, std::shared_ptr<TestNetworkNode>> m_nodes;
};

} // namespace Quasar
