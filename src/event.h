#pragma once

#include <eventpp/eventqueue.h>
#include <variant>

#include "quasar.pb.h"
#include "types.h"

namespace Quasar
{

enum class EventType
{
	MESSAGE,  // data: pair<Signature, MessageData>
	CALLBACK, // data: function<void()>
	TIMEOUT,  // data: Round
	ADVANCE,  // data: pair<Round, bool>
};

// TODO: should probably create new types for each of the possible EventData types.
using EventData = std::variant<std::monostate, std::pair<Signature, Proto::MessageData>, std::function<void()>, Round,
                               std::pair<Round, bool>>;
using EventQueue = eventpp::EventQueue<EventType, void(EventData)>;

} // namespace Quasar
