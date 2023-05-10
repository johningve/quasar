#include <fmt/format.h>
#include <google/protobuf/io/zero_copy_stream.h>

#include "exception.h"
#include "network.h"

namespace Quasar
{

ZMQNetwork::ZMQNetwork(int port) : m_context(1), m_listener(m_context, zmq::socket_type::pull)
{
	m_listener.bind(fmt::format("tcp://*:{}", port));
}

NetworkType ZMQNetwork::type()
{
	return NetworkType::POLLED;
}

void ZMQNetwork::poll(std::chrono::milliseconds duration)
{
	if (m_poller.empty())
	{
		m_poller.add(m_listener, zmq::event_flags::pollin,
		             [self = shared_from_this()](auto) { self->receive_message(); });
	}
	m_poller.wait(duration);
}

void ZMQNetwork::connect_to(const Identity &peer, const std::string &address)
{
	zmq::socket_t socket{m_context, zmq::socket_type::push};
	socket.connect(fmt::format("tcp://{}", address));
	m_connections.insert({peer, std::move(socket)});
}

void ZMQNetwork::send_message(const Identity &recipient, const Proto::Message &msg)
{
	auto entry = m_connections.find(recipient);
	if (entry == m_connections.end())
	{
		throw QUASAR_EXCEPTION_KIND(Exception::Kind::NOT_FOUND, "a connection to {:.8} was not found",
		                            recipient.to_hex_string());
	}

	auto &[_, socket] = *entry;

	socket.send(zmq::buffer(msg.SerializeAsString()));
}

void ZMQNetwork::broadcast_message(const Proto::Message &msg)
{
	for (auto &[id, _] : m_connections)
	{
		send_message(id, msg);
	}
}

void ZMQNetwork::set_message_handler(std::function<void(Proto::Message &)> handler)
{
	m_handler = handler;
}

int ZMQNetwork::size()
{
	return (int)m_connections.size();
}

std::vector<Identity> ZMQNetwork::connected_peers()
{
	std::vector<Identity> peers;
	peers.reserve(m_connections.size());

	for (auto &[peer, _] : m_connections)
	{
		peers.push_back(peer);
	}

	return peers;
}

void ZMQNetwork::receive_message()
{
	zmq::message_t msg_buf;
	auto res = m_listener.recv(msg_buf);
	if (res == std::nullopt)
	{
		throw QUASAR_EXCEPTION("unexpected nullopt");
	}

	Proto::Message msg;
	msg.ParseFromArray(msg_buf.data(), (int)msg_buf.size());

	m_handler(msg);
}

} // namespace Quasar