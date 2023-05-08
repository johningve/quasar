#pragma once

#include <botan/pubkey.h>

#include "types.h"

namespace Quasar
{

class Keystore
{
  public:
	explicit Keystore(std::shared_ptr<Botan::Private_Key> private_key);
	std::shared_ptr<Botan::Private_Key> private_key() const;
	const Identity &identity() const;
	std::shared_ptr<Botan::Public_Key> find_public_key(const Identity &identity) const;
	void add_public_key(const Identity &identity, const std::shared_ptr<Botan::Public_Key> &public_key);

  private:
	std::unordered_map<Identity, std::shared_ptr<Botan::Public_Key>> m_public_keys;
	std::shared_ptr<Botan::Private_Key> m_private_key;
	Identity m_identity;
};

} // namespace Quasar
