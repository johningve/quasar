#include "network.h"

namespace Quasar {
    Network::Network() : context(1) {}

    void Network::send_message(const Identity &recipient, const Proto::Message &msg) {
    }
} // Quasar