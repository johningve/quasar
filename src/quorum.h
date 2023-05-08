#pragma once

namespace Quasar
{

int num_faulty(int n)
{
	return (n - 1) / 3;
}

int quorum_size(int n)
{
	return n - num_faulty(n);
}

} // namespace Quasar