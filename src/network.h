#pragma once

#include <zmq.hpp>

#include "types.h"
#include "quasar.pb.h"

namespace Quasar {

    class INetwork {
    public:
        virtual void send_message(const Identity &recipient, const Proto::Message &msg) = 0;
    };

    class Network : INetwork {
    public:
        Network();

    private:
        void send_message(const Identity &recipient, const Proto::Message &msg) override;

    private:
        zmq::context_t context;


    };

} // Quasar
