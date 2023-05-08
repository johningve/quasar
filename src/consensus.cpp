#include "consensus.h"
#include "crypto.h"
#include "quorum.h"

namespace Quasar
{
Consensus::Consensus(const std::shared_ptr<EventQueue> &event_queue, const std::shared_ptr<Blockchain> &blockchain,
                     const std::shared_ptr<Keystore> &keystore, const std::shared_ptr<Network> &network,
                     const std::shared_ptr<Synchronizer> &synchronizer,
                     const std::shared_ptr<LeaderRotation> &leader_rotation,
                     const std::shared_ptr<spdlog::logger> &logger)
    : m_event_queue(event_queue), m_blockchain(blockchain), m_keystore(keystore), m_network(network),
      m_synchronizer(synchronizer), m_leader_rotation(leader_rotation), m_logger(logger),
      m_lock(std::make_shared<Block>(GENESIS))
{
}

void Consensus::init()
{
	m_event_queue->appendListener(EventType::MESSAGE, [self = shared_from_this()](const EventData &event) {
		auto &pair = std::get<std::pair<Signature, Proto::MessageData>>(event);
		self->handle_message(pair.first, pair.second);
	});
}

void Consensus::handle_message(const Signature &sig, const Proto::MessageData &msg)
{
	if (msg.has_proposal())
	{
		handle_proposal(sig, msg);
	}
	else if (msg.has_vote())
	{
		handle_vote(sig, msg);
	}
}

Proto::Message make_vote(const Signature &signature, Round round)
{
	Proto::Message msg;
	auto sig_ptr = msg.mutable_signature();
	*sig_ptr = signature.to_proto();
	auto data_ptr = msg.mutable_data();
	auto vote_ptr = data_ptr->mutable_vote();
	vote_ptr->set_round(round);
	return msg;
}

void Consensus::handle_proposal(const Signature &sig, const Proto::MessageData &msg)
{
	Block proposal{msg.proposal()};

	auto parent = m_blockchain->find(proposal.parent());
	if (!parent)
	{
		m_logger->warn("parent block with hash {:.8} was not found", proposal.parent().to_hex_string());
		return;
	}

	// check if the parent can replace the lock
	if (parent->round() <= m_lock->round())
	{
		m_logger->info("parent {:.8} does not rank higher than lock {:.8}", parent->hash().to_hex_string(),
		               m_lock->hash().to_hex_string());
		return;
	}

	// check if the parent is certified
	if (!Crypto::verify_certificate(proposal.certificate(), parent->to_proto().SerializeAsString(), *m_keystore))
	{
		m_logger->warn("proposal by {:.8} has invalid certificate", sig.signer().to_hex_string());
		return;
	}

	m_synchronizer->advance_to(parent->round() + 1);

	m_lock = parent;
	m_blockchain->add(proposal);

	auto grandparent = m_blockchain->find(parent->parent());
	if (grandparent)
	{
		m_blockchain->commit(grandparent);
	}
	else
	{
		m_logger->info("grandparent block with hash {:.8} was not found", parent->parent().to_hex_string());
	}

	auto vote = Crypto::sign(msg.proposal().SerializeAsString(), *m_keystore->private_key());
	auto vote_msg = make_vote(vote, m_synchronizer->round());

	if (m_leader_rotation->leader(m_synchronizer->round() + 1) == m_keystore->identity())
	{
		handle_vote(vote, vote_msg.data());
		return;
	}

	m_network->broadcast_message(vote_msg);
}

void Consensus::handle_vote(const Signature &sig, const Proto::MessageData &msg)
{
	Round round = (Round)msg.vote().round();
	if (m_synchronizer->round() > round)
	{
		// too old
		m_logger->debug("vote in round {} by {:.8} is too old", round, sig.signer().to_hex_string());
		return;
	}

	auto &votes = m_votes[round];
	votes.push_back(sig);

	if (votes.size() >= quorum_size(m_network->size()))
	{
		Certificate cert(votes);
		// clear votes so that we don't create the cert again if another vote shows up later
		votes.clear();

		// TODO: advance round, propose
	}
}

void Consensus::cleanup_votes(Round min_round)
{
	for (auto it = m_votes.begin(); it != m_votes.end();)
	{
		if (it->first < min_round)
		{
			m_votes.erase(it);
		}
		else
		{
			it++;
		}
	}
}

} // namespace Quasar