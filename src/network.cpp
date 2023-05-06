#include "network.h"

#include <utility>

namespace Quasar
{
ZMQNetwork::ZMQNetwork() : context(1)
{
}

void ZMQNetwork::send_message(const Identity &recipient, const Proto::Message &msg)
{
}

const char *NetworkError::what() const noexcept
{
	switch (m_kind)
	{
	case Kind::NOT_FOUND:
		return "not found";
	}
	return "unknown";
}

NetworkError::Kind NetworkError::kind() const
{
	return m_kind;
}

std::string NetworkError::detail() const
{
	return m_detail;
}

NetworkError::NetworkError(NetworkError::Kind m_kind, std::string m_detail)
    : m_kind(m_kind), m_detail(std::move(m_detail))
{
}

} // namespace Quasar