#pragma once

#include <fmt/format.h>
#include <stdexcept>

namespace Quasar
{

class Exception : public std::exception
{
  public:
	template <typename... Args>
	explicit Exception(fmt::format_string<Args...> fmt_string, Args &&...args)
	    : m_message(fmt::format(fmt_string, std::forward<Args>(args)...)), m_what(m_message)
	{
	}

	template <typename... Args>
	explicit Exception(int line, std::string file, fmt::format_string<Args...> fmt_string, Args &&...args)
	    : m_message(fmt::format(fmt_string, std::forward<Args>(args)...)),
	      m_what(fmt::format("{} ({}:{})", m_message, file, line))
	{
	}

	const char *what() const noexcept override;
	const std::string &message() const;

  private:
	std::string m_message;
	std::string m_what;
};

} // namespace Quasar

#define QUASAR_EXCEPTION(format, ...) Quasar::Exception(__LINE__, __FILE__, format __VA_OPT__(, ) __VA_ARGS__)