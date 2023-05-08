#include "leader_rotation.h"

namespace Quasar
{

Identity RoundRobinLeaderRotation::leader(std::vector<Identity> all_nodes, Round round)
{
	std::sort(all_nodes.begin(), all_nodes.end());
	return all_nodes[round % all_nodes.size()];
}

} // namespace Quasar