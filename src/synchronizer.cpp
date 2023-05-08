#include <cmath>

#include "crypto.h"
#include "quorum.h"
#include "synchronizer.h"

namespace Quasar
{

RoundDuration::RoundDuration(uint64_t history_size, double start_timeout, double max_timeout, double timeout_multiplier)
    : m_max_history_size(history_size), m_mean_duration(start_timeout), m_max_timeout(max_timeout),
      m_timeout_multiplier(timeout_multiplier)
{
}

void RoundDuration::round_started()
{
	m_measurement_start = std::chrono::system_clock::now();
}

void RoundDuration::round_succeeded()
{
	if (m_measurement_start.time_since_epoch() == std::chrono::system_clock::duration::zero())
	{
		// no measurement was started yet
		return;
	}

	double duration = std::chrono::duration<double>(m_measurement_start - std::chrono::system_clock::now()).count();

	m_count++;

	// Reset m2 occasionally such that we will pick up on changes in variance faster.
	// We store the m2 to prevM2, which will be used when calculating the variance.
	// This ensures that at least 'limit' measurements have contributed to the approximate variance.
	if (m_count % m_max_history_size == 0)
	{
		m_prev_mean_diff_2 = m_mean_diff_2;
		m_mean_diff_2 = 0;
	}

	double c;
	if (m_count > m_max_history_size)
	{
		c = (double)m_max_history_size;
		// discard one measurement
		m_mean_duration -= m_mean_duration / c;
	}
	else
	{
		c = (double)m_count;
	}

	// Welford's algorithm
	double d1 = duration - m_mean_duration;
	m_mean_duration += d1 / c;
	double d2 = duration - m_mean_duration;
	m_mean_diff_2 += d1 * d2;
}

void RoundDuration::round_timed_out()
{
	m_mean_duration *= m_timeout_multiplier;
}

std::chrono::duration<double> RoundDuration::expected_duration()
{
	double conf = 1.96; // 95% confidence
	double dev;

	if (m_count > 1)
	{
		auto c = (double)m_count;
		double m2 = m_mean_diff_2;
		if (m_count >= m_max_history_size)
		{
			c = m_max_history_size + m_count % m_max_history_size;
			m2 += m_prev_mean_diff_2;
		}
		dev = std::sqrt(m2 / c);
	}

	double duration = m_mean_duration + dev * conf;
	if (m_max_timeout > 0 && duration > m_max_timeout)
	{
		duration = m_max_timeout;
	}

	return std::chrono::duration<double>(duration);
}

Synchronizer::Synchronizer(const std::shared_ptr<EventQueue> &event_queue, const std::shared_ptr<Network> &network,
                           const std::shared_ptr<Keystore> &keystore, const std::shared_ptr<spdlog::logger> &logger)
    : m_round(0), m_event_queue(event_queue), m_network(network), m_keystore(keystore), m_logger(logger)
{
}

void Synchronizer::init()
{
	m_event_queue->appendListener(EventType::MESSAGE, [self = shared_from_this()](const EventData &event) {
		auto &pair = std::get<std::pair<Signature, Proto::MessageData>>(event);
		self->handle_message(pair.first, pair.second);
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

	if (timeout)
	{
		m_round_duration.round_timed_out();
	}
	else
	{
		m_round_duration.round_succeeded();
	}

	// TODO: cancel timers, etc.
	m_round = round;

	m_round_duration.round_started();
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
		Certificate cert(wishes);
		// clear wishes so that we don't create the cert again if another wish shows up later
		wishes.clear();

		// TODO: advance round, propose
	}
}

void Synchronizer::handle_advance(const Signature &sig, const Proto::MessageData &msg)
{
	auto round = (Round)msg.advance().wish().round();
	Certificate cert{msg.advance().certificate()};

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
	// TODO: propose
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

} // namespace Quasar