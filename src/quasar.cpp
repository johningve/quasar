#include <spdlog/sinks/stdout_color_sinks.h>
#include <utility>

#include "crypto.h"
#include "exception.h"
#include "quasar.h"

namespace Quasar
{

Quasar::Quasar(std::shared_ptr<Keystore> keystore, std::shared_ptr<Network> network,
               std::shared_ptr<LeaderRotation> leader_rotation)
    : m_keystore(std::move(keystore)), m_network(std::move(network)), m_logger(spdlog::stderr_color_mt("stderr")),
      m_leader_rotation(std::move(leader_rotation))
{
	m_consensus = std::make_shared<Consensus>(m_event_queue, m_blockchain, m_keystore, m_network, m_synchronizer,
	                                          m_leader_rotation, m_logger);
	m_consensus->init();

	// push network messages to event_queue
	m_network->set_message_handler(
	    [keystore = m_keystore, event_queue = m_event_queue, logger = m_logger](auto &message) {
		    Signature sig{message.signature()};

		    auto key = keystore->find_public_key(sig.signer());
		    if (key == nullptr)
		    {
			    logger->warn("public key with ID {:.8} not found", sig.signer().to_hex_string());
			    return;
		    }

		    if (!Crypto::verify(sig, message.data().SerializeAsString(), *key))
		    {
			    logger->warn("message received with invalid signature");
			    return;
		    }

		    event_queue->dispatch(EventType::MESSAGE, std::make_pair(sig, std::move(message.data())));
	    });

	// add event handler to execute callbacks
	m_event_queue->appendListener(EventType::CALLBACK,
	                              [](const EventData &event) { std::get<std::function<void()>>(event)(); });
}

} // namespace Quasar