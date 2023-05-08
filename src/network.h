#pragma once

#include <zmq.hpp>

#include "quasar.pb.h"
#include "types.h"

namespace Quasar
{

class Network
{
  public:
	virtual void send_message(const Identity &recipient, const Proto::Message &msg) = 0;
	virtual void broadcast_message(const Proto::Message &msg) = 0;
	virtual void set_message_handler(std::function<void(Proto::Message &)> handler) = 0;
	virtual int size() = 0;
};

class ZMQNetwork : Network
{
  public:
	ZMQNetwork();

  private:
	void send_message(const Identity &recipient, const Proto::Message &msg) override;

  private:
	zmq::context_t context;
};

} // namespace Quasar
