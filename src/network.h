#pragma once

#include <zmq.hpp>
#include <zmq_addon.hpp>

#include "quasar.pb.h"
#include "types.h"

namespace Quasar
{

enum class NetworkType
{
	UNSPECIFIED,
	POLLED,
};

class Network
{
  public:
	virtual NetworkType type() = 0;
	virtual void send_message(const Identity &recipient, const Proto::Message &msg) = 0;
	virtual void broadcast_message(const Proto::Message &msg) = 0;
	virtual void set_message_handler(std::function<void(Proto::Message &)> handler) = 0;
	virtual int size() = 0;
	virtual std::vector<Identity> connected_peers() = 0;
};

class PolledNetwork : public Network
{
  public:
	virtual void poll(std::chrono::milliseconds duration) = 0;
};

class ZMQNetwork : public PolledNetwork, public std::enable_shared_from_this<ZMQNetwork>
{
  public:
	ZMQNetwork(int port);

	NetworkType type() override;

	void connect_to(const Identity &peer, const std::string &address);

	void send_message(const Identity &recipient, const Proto::Message &msg) override;
	void broadcast_message(const Proto::Message &msg) override;
	void set_message_handler(std::function<void(Proto::Message &)> handler) override;
	int size() override;
	std::vector<Identity> connected_peers() override;

	void poll(std::chrono::milliseconds duration) override;

  private:
	void receive_message();

	zmq::context_t m_context;
	zmq::socket_t m_listener;
	zmq::active_poller_t m_poller;
	std::unordered_map<Identity, zmq::socket_t> m_connections;
	std::function<void(Proto::Message &)> m_handler;
};

} // namespace Quasar
