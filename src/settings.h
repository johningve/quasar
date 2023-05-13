#pragma once

#include "types.h"

namespace Quasar
{

class Settings
{
  private:
	class Setting
	{
	  protected:
		virtual void apply(Settings &settings) = 0;
	};

	friend Setting;

  public:
	template <typename... Args> explicit Settings(Args... args) : m_consensus(), m_round_duration()
	{
		([&] { args.apply(*this); }(), ...);
	}

	class Consensus
	{
	  public:
		Consensus() : m_allow_empty_blocks(true)
		{
		}

		class AllowEmptyBlocks : Setting
		{
		  public:
			explicit AllowEmptyBlocks(bool choice) : m_choice(choice)
			{
			}

		  private:
			void apply(Settings &settings) override
			{
				settings.m_consensus.m_allow_empty_blocks = m_choice;
			}

			bool m_choice;
		};

		bool allow_empty_blocks() const
		{
			return m_allow_empty_blocks;
		}

	  private:
		bool m_allow_empty_blocks;
	};

	class RoundDuration
	{
	  public:
		RoundDuration() : m_history_size(100), m_start_timeout(100), m_max_timeout(1000), m_timeout_multiplier(1.2)
		{
		}

		class HistorySize : Setting
		{
		  public:
			explicit HistorySize(uint64_t history_size) : m_history_size(history_size)
			{
			}

		  private:
			void apply(Settings &settings) override
			{
				settings.m_round_duration.m_history_size = m_history_size;
			}

			uint64_t m_history_size;
		};

		class StartTimeout : Setting
		{
		  public:
			explicit StartTimeout(std::chrono::milliseconds start_timeout) : m_start_timeout(start_timeout)
			{
			}

		  private:
			void apply(Settings &settings) override
			{
				settings.m_round_duration.m_start_timeout = m_start_timeout;
			}

			std::chrono::milliseconds m_start_timeout;
		};

		class MaxTimeout : Setting
		{
		  public:
			explicit MaxTimeout(std::chrono::milliseconds max_timeout) : m_max_timeout(max_timeout)
			{
			}

		  private:
			void apply(Settings &settings) override
			{
				settings.m_round_duration.m_max_timeout = m_max_timeout;
			}

			std::chrono::milliseconds m_max_timeout;
		};

		class TimeoutMultiplier : Setting
		{
		  public:
			explicit TimeoutMultiplier(double timeout_multiplier) : m_timeout_multiplier(timeout_multiplier)
			{
			}

		  private:
			void apply(Settings &settings) override
			{
				settings.m_round_duration.m_timeout_multiplier = m_timeout_multiplier;
			}

			double m_timeout_multiplier;
		};

		uint64_t history_size() const
		{
			return m_history_size;
		}

		std::chrono::milliseconds start_timeout() const
		{
			return m_start_timeout;
		}

		std::chrono::milliseconds max_timeout() const
		{
			return m_max_timeout;
		}

		double timeout_multiplier() const
		{
			return m_timeout_multiplier;
		}

	  private:
		uint64_t m_history_size;
		std::chrono::milliseconds m_start_timeout;
		std::chrono::milliseconds m_max_timeout;
		double m_timeout_multiplier;
	};

	Consensus consensus() const
	{
		return m_consensus;
	}

	RoundDuration round_duration() const
	{
		return m_round_duration;
	}

  private:
	Consensus m_consensus;
	RoundDuration m_round_duration;
};

} // namespace Quasar
