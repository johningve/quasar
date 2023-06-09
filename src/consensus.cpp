#include "consensus.h"
#include "crypto.h"
#include "quorum.h"

namespace Quasar
{

Consensus::Consensus(const Settings::Consensus &settings, const std::shared_ptr<EventQueue> &event_queue,
                     const std::shared_ptr<Blockchain> &blockchain, const std::shared_ptr<Keystore> &keystore,
                     const std::shared_ptr<Network> &network, const std::shared_ptr<Synchronizer> &synchronizer,
                     const std::shared_ptr<LeaderRotation> &leader_rotation,
                     const std::shared_ptr<spdlog::logger> &logger)
    : m_settings(settings), m_event_queue(event_queue), m_blockchain(blockchain), m_keystore(keystore),
      m_network(network), m_synchronizer(synchronizer), m_leader_rotation(leader_rotation), m_logger(logger),
      m_lock(std::make_shared<Block>(GENESIS)), m_high_cert({GENESIS_CERT, GENESIS.hash()}), m_next_vote_round(1)
{
}

void Consensus::init()
{
	m_event_queue->appendListener(EventType::MESSAGE, [self = shared_from_this()](const EventData &event) {
		auto &pair = std::get<std::pair<Signature, Proto::MessageData>>(event);
		self->handle_message(pair.first, pair.second);
	});

	m_event_queue->appendListener(EventType::TIMEOUT, [self = shared_from_this()](const EventData &event) {
		self->stop_voting(std::get<Round>(event));
	});

	m_event_queue->appendListener(EventType::ADVANCE, [self = shared_from_this()](const EventData &event) {
		auto [round, timeout] = std::get<std::pair<Round, bool>>(event);

		auto all_nodes = self->m_network->connected_peers();
		all_nodes.push_back(self->m_keystore->identity());

		if (self->m_leader_rotation->leader(all_nodes, round) == self->m_keystore->identity())
		{
			if (!timeout)
			{
				self->make_proposal();
			}
			// wait for about one theta (that is, the message passing delay)
			self->m_timer_manager.add(self->m_synchronizer->round_duration() / 2, [self](auto timer_id) {
				self->m_timer_manager.remove(timer_id);
				self->m_event_queue->dispatch(EventType::CALLBACK, [self]() { self->make_proposal(); });
			});
		}
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

Proto::Message make_vote(const Signature &signature, Hash hash)
{
	Proto::Message msg;
	auto sig_ptr = msg.mutable_signature();
	*sig_ptr = signature.to_proto();
	auto data_ptr = msg.mutable_data();
	auto vote_ptr = data_ptr->mutable_vote();
	vote_ptr->set_block_hash(hash.to_byte_string());
	return msg;
}

void Consensus::handle_proposal(const Signature &sig, const Proto::MessageData &msg)
{
	const Block proposal{msg.proposal()};

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

	m_lock = parent;
	m_high_cert = BlockCertificate{proposal.certificate(), proposal.parent()};
	m_blockchain->add(proposal);

	m_synchronizer->advance_to(parent->round() + 1);

	auto grandparent = m_blockchain->find(parent->parent());
	if (grandparent)
	{
		m_blockchain->commit(grandparent);
	}
	else
	{
		m_logger->info("grandparent block with hash {:.8} was not found", parent->parent().to_hex_string());
	}

	if (m_next_vote_round > proposal.round())
	{
		m_logger->info("voting ended for round {}", proposal.round());
		return;
	}

	stop_voting(proposal.round());

	auto vote = Crypto::sign(proposal.hash().to_byte_string(), *m_keystore->private_key());
	auto vote_msg = make_vote(vote, proposal.hash());

	auto all_nodes = m_network->connected_peers();
	all_nodes.push_back(m_keystore->identity());
	auto leader = m_leader_rotation->leader(std::move(all_nodes), m_synchronizer->round() + 1);

	if (leader == m_keystore->identity())
	{
		handle_vote(vote, vote_msg.data());
		return;
	}

	m_network->send_message(leader, vote_msg);
}

void Consensus::handle_vote(const Signature &sig, const Proto::MessageData &msg)
{
	auto block_hash = Hash::from_byte_string(msg.vote().block_hash());
	auto block = m_blockchain->find(block_hash);
	if (block == nullptr)
	{
		m_logger->warn("could not find block {:.8} referenced by vote", block_hash.to_hex_string());
		return;
	}
	auto round = block->round();

	if (m_synchronizer->round() > round)
	{
		// too old
		m_logger->debug("vote in round {} by {:.8} is too old", round, sig.signer().to_hex_string());
		return;
	}

	auto &votes = m_votes[round];
	votes.push_back(sig);

	if (votes.size() < quorum_size(m_network->size()))
	{
		return;
	}

	const Certificate cert(votes);
	// clear votes so that we don't create the cert again if another vote shows up later
	votes.clear();

	auto high_cert_block = m_blockchain->find(m_high_cert.block_hash);
	if (high_cert_block == nullptr)
	{
		m_logger->error("could not find the block {:.8} certified by high_cert",
		                m_high_cert.block_hash.to_hex_string());
		return;
	}

	if (round <= high_cert_block->round())
	{
		m_logger->error("created cert (round={}) ranks lower than high_cert (round={})", round,
		                high_cert_block->round());
		return;
	}

	m_high_cert = {cert, block_hash};

	m_synchronizer->advance_to(round + 1);

	cleanup_votes(round + 1);
}

void Consensus::make_proposal()
{
	// TODO: get a payload from Mempool
	const Block proposal{m_high_cert.block_hash, m_high_cert.certificate, m_synchronizer->round(), {}};

	Proto::Message msg;
	auto msg_data_ptr = msg.mutable_data();
	auto proposal_ptr = msg_data_ptr->mutable_proposal();
	*proposal_ptr = proposal.to_proto();

	auto sig = Crypto::sign(msg_data_ptr->SerializeAsString(), *m_keystore->private_key());
	auto msg_sig_ptr = msg.mutable_signature();
	*msg_sig_ptr = sig.to_proto();

	m_network->broadcast_message(msg);
	handle_proposal(sig, *msg_data_ptr);
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

void Consensus::stop_voting(Round round)
{
	if (m_next_vote_round <= round)
	{
		m_next_vote_round = round + 1;
	}
}

} // namespace Quasar