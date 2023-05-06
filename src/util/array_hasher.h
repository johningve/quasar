#pragma once

#include <array>
#include <cstddef>
#include <functional>

namespace std
{
template <typename T, size_t N> struct hash<array<T, N>>
{
	size_t operator()(const array<T, N> &a) const noexcept
	{
		hash<T> hasher;
		size_t h = 0;
		for (size_t i = 0; i < N; i++)
		{
			h = h * 31 + hasher(a[i]);
		}
		return h;
	}
};

} // namespace std
