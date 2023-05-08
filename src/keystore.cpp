#include "keystore.h"
#include "crypto.h"

#include <utility>

namespace Quasar
{

Keystore::Keystore(std::shared_ptr<Botan::Private_Key> private_key) : m_private_key(std::move(private_key))
{
	m_identity = Crypto::hash(m_private_key->public_key_bits());
}

std::shared_ptr<Botan::Private_Key> Keystore::private_key() const
{
	return m_private_key;
}

const Identity &Keystore::identity() const
{
	return m_identity;
}

std::shared_ptr<Botan::Public_Key> Keystore::find_public_key(const Identity &identity) const
{
	auto elem = m_public_keys.find(identity);
	if (elem == m_public_keys.end())
	{
		return nullptr;
	}
	return elem->second;
}

void Keystore::add_public_key(const Identity &identity, const std::shared_ptr<Botan::Public_Key> &public_key)
{
	m_public_keys.insert({identity, public_key});
}

} // namespace Quasar