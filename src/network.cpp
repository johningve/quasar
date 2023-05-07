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

} // namespace Quasar