#include <cmath>

#include "crypto.h"
#include "round_duration.h"

Quasar::RoundDuration::RoundDuration() : RoundDuration(100, 100, 1000, 1.2)
{
}

Quasar::RoundDuration::RoundDuration(const Quasar::Settings::RoundDuration &settings)
{
	m_max_history_size = settings.history_size();
	m_mean_duration =
	    std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(settings.start_timeout()).count();
	m_max_timeout =
	    std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(settings.max_timeout()).count();
	m_timeout_multiplier = settings.timeout_multiplier();
}

Quasar::RoundDuration::RoundDuration(uint64_t history_size, double start_timeout, double max_timeout,
                                     double timeout_multiplier)
    : m_max_history_size(history_size), m_mean_duration(start_timeout), m_max_timeout(max_timeout),
      m_timeout_multiplier(timeout_multiplier)
{
}

void Quasar::RoundDuration::round_started()
{
	m_measurement_start = std::chrono::system_clock::now();
}

void Quasar::RoundDuration::round_succeeded()
{
	if (m_measurement_start.time_since_epoch() == std::chrono::system_clock::duration::zero())
	{
		// no measurement was started yet
		return;
	}

	double duration =
	    std::chrono::duration<double, std::milli>(m_measurement_start - std::chrono::system_clock::now()).count();

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

void Quasar::RoundDuration::round_timed_out()
{
	m_mean_duration *= m_timeout_multiplier;
}

std::chrono::milliseconds Quasar::RoundDuration::expected_duration() const
{
	double conf = 1.96; // 95% confidence
	double dev = 0;

	if (m_count > 1)
	{
		auto c = (double)m_count;
		double m2 = m_mean_diff_2;
		if (m_count >= m_max_history_size)
		{
			c = (double)(m_max_history_size + m_count % m_max_history_size);
			m2 += m_prev_mean_diff_2;
		}
		dev = sqrt(m2 / c);
	}

	double duration = m_mean_duration + dev * conf;
	if (m_max_timeout > 0 && duration > m_max_timeout)
	{
		duration = m_max_timeout;
	}

	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<double, std::milli>(duration));
}
