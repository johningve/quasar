#pragma once

#include <spdlog/logger.h>

#include "blockchain.h"
#include "event.h"
#include "keystore.h"
#include "leader_rotation.h"
#include "network.h"
#include "synchronizer.h"

namespace Quasar
{

struct BlockCertificate
{
	Certificate certificate;
	Hash block_hash;
};

class Consensus : public std::enable_shared_from_this<Consensus>
{
  public:
	Consensus(const std::shared_ptr<EventQueue> &event_queue, const std::shared_ptr<Blockchain> &blockchain,
	          const std::shared_ptr<Keystore> &keystore, const std::shared_ptr<Network> &network,
	          const std::shared_ptr<Synchronizer> &synchronizer, const std::shared_ptr<LeaderRotation> &leader_rotation,
	          const std::shared_ptr<spdlog::logger> &logger);

	// init sets up event handlers
	void init();

  private:
	void handle_message(const Signature &sig, const Proto::MessageData &msg);
	void handle_proposal(const Signature &sig, const Proto::MessageData &msg);
	void handle_vote(const Signature &sig, const Proto::MessageData &msg);

	void make_proposal();

	void stop_voting(Round round);
	void cleanup_votes(Round min_round);

	std::shared_ptr<EventQueue> m_event_queue;
	std::shared_ptr<Blockchain> m_blockchain;
	std::shared_ptr<Keystore> m_keystore;
	std::shared_ptr<Network> m_network;
	std::shared_ptr<Synchronizer> m_synchronizer;
	std::shared_ptr<LeaderRotation> m_leader_rotation;
	std::shared_ptr<spdlog::logger> m_logger;

	Round m_next_vote_round;
	std::shared_ptr<Block> m_lock;
	BlockCertificate m_high_cert;
	std::unordered_map<Round, std::vector<Signature>> m_votes;
};

} // namespace Quasar
