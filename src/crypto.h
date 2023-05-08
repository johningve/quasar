#pragma once

#include <botan/ecdsa.h>
#include <botan/pubkey.h>

#include "keystore.h"
#include "types.h"

namespace Quasar::Crypto
{
Hash hash(const std::vector<uint8_t> &data);
Hash hash(const std::string &data);

bool verify(const Signature &signature, const std::string &message, const Botan::Public_Key &key);
bool verify_certificate(const Certificate &cert, const std::string &message, const Keystore &keystore);

Signature sign(const std::string &message, const Botan::Private_Key &key);

} // namespace Quasar::Crypto
