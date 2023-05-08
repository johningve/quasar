#pragma once

#include <chrono>
#include <cpptime.h>
#include <spdlog/logger.h>

#include "event.h"
#include "keystore.h"
#include "network.h"
#include "types.h"

namespace Quasar
{

// RoundDuration tracks the duration of rounds and calculates round timeout.
class RoundDuration
{
  public:
	RoundDuration(uint64_t history_size, double start_timeout, double max_timeout, double timeout_multiplier);
	void round_started();
	void round_succeeded();
	void round_timed_out();
	std::chrono::duration<double> expected_duration();

  private:
	double m_timeout_multiplier;
	double m_max_timeout;
	uint64_t m_max_history_size;

	std::chrono::time_point<std::chrono::system_clock> m_measurement_start;
	double m_mean_duration;
	double m_mean_diff_2;
	double m_prev_mean_diff_2;
	uint64_t m_count;
};

// Synchronizer synchronizes entry to rounds.
// It is inactive during the normal view protocol, but activates one a timeout occurs.
class Synchronizer : public std::enable_shared_from_this<Synchronizer>
{
  public:
	Synchronizer(const std::shared_ptr<EventQueue> &event_queue, const std::shared_ptr<Network> &network,
	             const std::shared_ptr<Keystore> &keystore, const std::shared_ptr<spdlog::logger> &logger);
	void init();

	Round round() const;
	// advance_to should be called by the view protocol upon obtaining a certificate for round-1
	void advance_to(Round round, bool timeout = false);

  private:
	void handle_message(const Signature &sig, const Proto::MessageData &msg);
	void handle_wish(const Signature &sig, const Proto::MessageData &msg);
	void handle_advance(const Signature &sig, const Proto::MessageData &msg);

	void cleanup_wishes(Round min_round);

	std::shared_ptr<EventQueue> m_event_queue;
	std::shared_ptr<Network> m_network;
	std::shared_ptr<Keystore> m_keystore;
	std::shared_ptr<spdlog::logger> m_logger;

	RoundDuration m_round_duration;
	CppTime::Timer m_timer;
	Round m_round;

	std::unordered_map<Round, std::vector<Signature>> m_wishes;
};

} // namespace Quasar
