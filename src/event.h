#pragma once

#include <eventpp/eventqueue.h>
#include <variant>

#include "quasar.pb.h"
#include "types.h"

namespace Quasar
{

enum class EventType
{
	MESSAGE,
	CALLBACK,
};

using EventData = std::variant<std::monostate, std::pair<Signature, Proto::MessageData>, std::function<void()>>;
using EventQueue = eventpp::EventQueue<EventType, void(EventData)>;

} // namespace Quasar
