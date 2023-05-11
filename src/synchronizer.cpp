#include "synchronizer.h"
#include "crypto.h"
#include "quorum.h"

namespace Quasar
{

Synchronizer::Synchronizer(const RoundDuration &round_duration, const std::shared_ptr<EventQueue> &event_queue,
                           const std::shared_ptr<Network> &network, const std::shared_ptr<Keystore> &keystore,
                           const std::shared_ptr<spdlog::logger> &logger)
    : m_round(0), m_round_duration(round_duration), m_event_queue(event_queue), m_network(network),
      m_keystore(keystore), m_logger(logger), m_timeout_timer(0)
{
}

void Synchronizer::init()
{
	m_event_queue->appendListener(EventType::MESSAGE, [self = shared_from_this()](const EventData &event) {
		auto &pair = std::get<std::pair<Signature, Proto::MessageData>>(event);
		self->handle_message(pair.first, pair.second);
	});

	m_event_queue->appendListener(EventType::TIMEOUT, [self = shared_from_this()](const EventData &event) {
		auto round = std::get<Round>(event);
		self->handle_timeout(round);
	});
}

Round Synchronizer::round() const
{
	return m_round;
}

void Synchronizer::advance_to(Round round, bool timeout)
{
	if (round <= m_round)
	{
		return;
	}

	stop_timeout_timer();

	if (timeout)
	{
		m_round_duration.round_timed_out();
	}
	else
	{
		m_round_duration.round_succeeded();
	}

	m_round = round;

	start_timeout_timer();

	m_round_duration.round_started();

	m_event_queue->dispatch(EventType::ADVANCE, std::make_pair(round, timeout));
}

std::chrono::duration<double> Synchronizer::round_duration() const
{
	return m_round_duration.expected_duration();
}

void Synchronizer::handle_message(const Signature &sig, const Proto::MessageData &msg)
{
	if (msg.has_wish())
	{
		handle_wish(sig, msg);
	}
	else if (msg.has_advance())
	{
		handle_advance(sig, msg);
	}
}

void Synchronizer::handle_wish(const Signature &sig, const Proto::MessageData &msg)
{
	auto round = (Round)msg.wish().round();
	if (m_round >= round)
	{
		// too old
		m_logger->debug("wish for round {} by {:.8}, but already in round {}", round, sig.signer().to_hex_string(),
		                m_round);
		return;
	}

	auto &wishes = m_wishes[round];
	wishes.push_back(sig);

	if (wishes.size() >= quorum_size(m_network->size()))
	{
		const Certificate cert(wishes);
		// clear wishes so that we don't create the cert again if another wish shows up later
		wishes.clear();

		perform_advance(cert, round);
	}
}

void Synchronizer::handle_advance(const Signature &sig, const Proto::MessageData &msg)
{
	auto round = (Round)msg.advance().wish().round();
	const Certificate cert{msg.advance().certificate()};

	if (m_round >= round)
	{
		m_logger->debug("timeout certificate for {} from {:.8}, but already in round {}", round,
		                sig.signer().to_hex_string(), m_round);
		return;
	}

	if (!Crypto::verify_certificate(cert, msg.advance().wish().SerializeAsString(), *m_keystore))
	{
		m_logger->warn("timeout certificate received from {:.8} is invalid", sig.signer().to_hex_string());
		return;
	}

	advance_to(round);
	cleanup_wishes(round);
}

void Synchronizer::handle_timeout(Quasar::Round round)
{
	Proto::Wish wish;
	wish.set_round(round);
	auto sig = Crypto::sign(wish.SerializeAsString(), *m_keystore->private_key());

	Proto::Message msg;
	auto sig_ptr = msg.mutable_signature();
	*sig_ptr = sig.to_proto();

	auto data_ptr = msg.mutable_data();
	auto wish_ptr = data_ptr->mutable_wish();
	*wish_ptr = wish;

	m_network->broadcast_message(msg);
	handle_wish(sig, *data_ptr);
}

void Synchronizer::cleanup_wishes(Round min_round)
{
	for (auto it = m_wishes.begin(); it != m_wishes.end();)
	{
		if (it->first < min_round)
		{
			m_wishes.erase(it);
		}
		else
		{
			it++;
		}
	}
}

void Synchronizer::start_timeout_timer()
{
	auto duration = m_round_duration.expected_duration();

	m_timeout_timer = m_timer_manager.add(duration, [self = shared_from_this(), round = m_round](auto timer_id) {
		// prevent this timer from executing again
		self->m_timer_manager.remove(timer_id);

		self->m_event_queue->dispatch(EventType::TIMEOUT, round);
	});
}

void Synchronizer::stop_timeout_timer()
{
	m_timer_manager.remove(m_timeout_timer);
}

void Synchronizer::perform_advance(const Certificate &cert, Round round)
{
	Proto::Message msg;
	auto msg_data_ptr = msg.mutable_data();
	auto advance_ptr = msg_data_ptr->mutable_advance();

	auto cert_ptr = advance_ptr->mutable_certificate();
	*cert_ptr = cert.to_proto();

	auto wish_ptr = advance_ptr->mutable_wish();
	wish_ptr->set_round(round);

	auto sig = Crypto::sign(msg_data_ptr->SerializeAsString(), *m_keystore->private_key());
	auto msg_sig_ptr = msg.mutable_signature();
	*msg_sig_ptr = sig.to_proto();

	m_network->broadcast_message(msg);
	handle_advance(sig, *msg_data_ptr);
}

} // namespace Quasar