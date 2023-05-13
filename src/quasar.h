#pragma once

#include <botan/pubkey.h>
#include <memory>
#include <spdlog/spdlog.h>
#include <variant>

#include "blockchain.h"
#include "consensus.h"
#include "event.h"
#include "keystore.h"
#include "leader_rotation.h"
#include "network.h"
#include "settings.h"
#include "synchronizer.h"

namespace Quasar
{

class Quasar
{
  public:
	Quasar(const Settings &settings, std::shared_ptr<Keystore> keystore, std::shared_ptr<Network> network,
	       std::shared_ptr<LeaderRotation> leader_rotation);

	void run();
	void stop();

  private:
	std::shared_ptr<EventQueue> m_event_queue;
	std::shared_ptr<Blockchain> m_blockchain;
	std::shared_ptr<Keystore> m_keystore;
	std::shared_ptr<Network> m_network;
	std::shared_ptr<Synchronizer> m_synchronizer;
	std::shared_ptr<Consensus> m_consensus;
	std::shared_ptr<spdlog::logger> m_logger;
	std::shared_ptr<LeaderRotation> m_leader_rotation;

	bool m_stopped;
};

} // namespace Quasar
