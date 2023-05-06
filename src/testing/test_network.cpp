#include <fmt/core.h>

#include "test_network.h"

namespace Quasar
{

void TestNetwork::send_message(const Identity &recipient, const Proto::Message &msg)
{
	auto conn_entry = m_connections.find(recipient);
	if (conn_entry == m_connections.end())
	{
		throw NetworkError{NetworkError::Kind::NOT_FOUND,
		                   fmt::format("a connection to {:.8} was not found", recipient.to_string())};
	}
	// create a copy so that we can allow modification
	auto my_msg = Proto::Message{msg};
	conn_entry->second->handle_message(my_msg);
}

void TestNetwork::broadcast_message(const Proto::Message &msg)
{
	std::for_each(m_connections.begin(), m_connections.end(),
	              [this, msg](auto el) { this->send_message(el.first, msg); });
}

void TestNetwork::set_message_handler(std::function<void(Proto::Message &)> handler)
{
	m_message_handler = std::move(handler);
}

void TestNetwork::add_connection(const Identity &id, std::shared_ptr<TestNetwork> network)
{
	m_connections.insert({id, network});
}

void TestNetwork::handle_message(Proto::Message &msg) const
{
	if (m_message_handler)
	{
		m_message_handler(msg);
	}
}

} // namespace Quasar