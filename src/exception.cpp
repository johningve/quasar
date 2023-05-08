#include "exception.h"

namespace Quasar
{

const char *Exception::what() const noexcept
{
	return m_what.c_str();
}

const std::string &Exception::message() const
{
	return m_message;
}

Exception::Kind Exception::kind() const
{
	return m_kind;
}

} // namespace Quasar