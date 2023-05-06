#pragma once

#include <botan/ecdsa.h>
#include <botan/pubkey.h>

#include "types.h"

namespace Quasar
{
class Crypto
{
  public:
	static Hash hash(const std::vector<uint8_t> &data);
	static Hash hash(const std::string &data);

	static bool verify(const Signature &signature, const std::vector<uint8_t> &message, const Botan::Public_Key &key);

	explicit Crypto(Botan::ECDSA_PrivateKey key);

	Signature sign(const std::vector<uint8_t> &message) const;

  private:
	Botan::ECDSA_PrivateKey m_key;
};
} // namespace Quasar
