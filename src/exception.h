#pragma once

#include <fmt/format.h>
#include <stdexcept>

namespace Quasar
{

class Exception : public std::exception
{
  public:
	enum class Kind
	{
		UNSPECIFIED,
		NOT_FOUND
	};

	template <typename... Args>
	explicit Exception(int line, std::string file, fmt::format_string<Args...> fmt_string, Args &&...args)
	    : m_kind(Kind::UNSPECIFIED), m_message(fmt::format(fmt_string, std::forward<Args>(args)...)),
	      m_what(fmt::format("{} ({}:{})", m_message, file, line))
	{
	}

	template <typename... Args>
	explicit Exception(Kind kind, int line, std::string file, fmt::format_string<Args...> fmt_string, Args &&...args)
	    : m_kind(kind), m_message(fmt::format(fmt_string, std::forward<Args>(args)...)),
	      m_what(fmt::format("{} ({}:{})", m_message, file, line))
	{
	}

	Kind kind() const;
	const char *what() const noexcept override;
	const std::string &message() const;

  private:
	Kind m_kind;
	std::string m_message;
	std::string m_what;
};

} // namespace Quasar

#define QUASAR_EXCEPTION(format, ...) ::Quasar::Exception(__LINE__, __FILE__, format __VA_OPT__(, ) __VA_ARGS__)
#define QUASAR_EXCEPTION_KIND(kind, format, ...)                                                                       \
	::Quasar::Exception(kind, __LINE__, __FILE__, format __VA_OPT__(, ) __VA_ARGS__)
