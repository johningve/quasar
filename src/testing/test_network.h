#pragma once

#include "../network.h"

namespace Quasar
{

class TestNetwork : Network
{
  public:
	void send_message(const Identity &recipient, const Proto::Message &msg) override;
	void broadcast_message(const Proto::Message &msg) override;
	void set_message_handler(std::function<void(Proto::Message &)> handler) override;

	void add_connection(const Identity &id, std::shared_ptr<TestNetwork> network);
	void handle_message(Proto::Message &msg) const;

  private:
	std::function<void(Proto::Message &)> m_message_handler;
	std::unordered_map<Identity, std::shared_ptr<TestNetwork>> m_connections;
};

} // namespace Quasar
