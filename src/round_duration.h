#pragma once

#include <chrono>

namespace Quasar
{

// RoundDuration tracks the duration of rounds and calculates round timeout.
class RoundDuration
{
  public:
	RoundDuration();
	RoundDuration(uint64_t history_size, double start_timeout, double max_timeout, double timeout_multiplier);
	void round_started();
	void round_succeeded();
	void round_timed_out();
	std::chrono::duration<double> expected_duration() const;

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

} // namespace Quasar