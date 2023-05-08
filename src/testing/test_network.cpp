#include <utility>

#include "../exception.h"
#include "test_network.h"

namespace Quasar
{

TestNetworkNode::TestNetworkNode(Identity identity, std::shared_ptr<TestNetwork> m_network)
    : m_id(identity), m_network(std::move(m_network))
{
}

void TestNetworkNode::send_message(const Identity &recipient, const Proto::Message &msg)
{
	m_network->send_message(recipient, msg);
}

void TestNetworkNode::broadcast_message(const Proto::Message &msg)
{
	m_network->broadcast_message(m_id, msg);
}

void TestNetworkNode::set_message_handler(std::function<void(Proto::Message &)> handler)
{
	m_message_handler = std::move(handler);
}

void TestNetworkNode::add_message(const Proto::Message &msg)
{
	m_message_queue.push(msg);
}

void TestNetworkNode::handle_message()
{
	if (m_message_queue.empty())
	{
		return;
	}

	auto msg = m_message_queue.front();
	m_message_queue.pop();

	if (m_message_handler)
	{
		m_message_handler(msg);
	}
}

int TestNetworkNode::size()
{
	return m_network->size();
}

std::shared_ptr<TestNetworkNode> TestNetwork::create_node(const Identity &id)
{
	auto node = std::make_shared<TestNetworkNode>(id, shared_from_this());
	m_nodes.insert({id, node});
	return node;
}

void TestNetwork::send_message(const Identity &recipient, const Proto::Message &msg)
{
	auto conn_entry = m_nodes.find(recipient);
	if (conn_entry == m_nodes.end())
	{
		throw QUASAR_EXCEPTION_KIND(Exception::Kind::NOT_FOUND, "a connection to {:.8} was not found",
		                            recipient.to_hex_string());
	}
	conn_entry->second->add_message(msg);
}

void TestNetwork::broadcast_message(const Identity &id_from, const Proto::Message &msg)
{
	for (auto [id, node] : m_nodes)
	{
		if (id == id_from)
		{
			continue;
		}

		node->add_message(msg);
	}
}

int TestNetwork::size()
{
	return (int)m_nodes.size();
}

void TestNetwork::run_for(int ticks)
{
	for (int i = 0; i < ticks; i++)
	{
		run_single();
	}
}

void TestNetwork::run_single()
{
	for (auto [_, node] : m_nodes)
	{
		node->handle_message();
	}
}

} // namespace Quasar