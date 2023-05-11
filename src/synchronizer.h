#pragma once

#include <cpptime.h>
#include <spdlog/logger.h>

#include "event.h"
#include "keystore.h"
#include "network.h"
#include "round_duration.h"
#include "types.h"

namespace Quasar
{

// Synchronizer synchronizes entry to rounds.
// It is inactive during the normal view protocol, but activates one a timeout occurs.
class Synchronizer : public std::enable_shared_from_this<Synchronizer>
{
  public:
	Synchronizer(const RoundDuration &round_duration, const std::shared_ptr<EventQueue> &event_queue,
	             const std::shared_ptr<Network> &network, const std::shared_ptr<Keystore> &keystore,
	             const std::shared_ptr<spdlog::logger> &logger);
	void init();

	Round round() const;
	// advance_to should be called by the view protocol upon obtaining a certificate for round-1
	void advance_to(Round round, bool timeout = false);

	std::chrono::duration<double> round_duration() const;

  private:
	void handle_message(const Signature &sig, const Proto::MessageData &msg);
	void handle_wish(const Signature &sig, const Proto::MessageData &msg);
	void handle_advance(const Signature &sig, const Proto::MessageData &msg);

	void handle_timeout(Round round);

	void cleanup_wishes(Round min_round);

	void start_timeout_timer();
	void stop_timeout_timer();

	void perform_advance(const Certificate &cert, Round round);

	std::shared_ptr<EventQueue> m_event_queue;
	std::shared_ptr<Network> m_network;
	std::shared_ptr<Keystore> m_keystore;
	std::shared_ptr<spdlog::logger> m_logger;

	RoundDuration m_round_duration;
	CppTime::Timer m_timer_manager;

	CppTime::timer_id m_timeout_timer;
	Round m_round;

	std::unordered_map<Round, std::vector<Signature>> m_wishes;
};

} // namespace Quasar
