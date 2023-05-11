#pragma once

#include <botan/ecdsa.h>
#include <botan/hash.h>
#include <botan/pubkey.h>

#include "keystore.h"
#include "types.h"

namespace Quasar::Crypto
{

template <typename T> Hash hash(T data)
{
	Hash hash{};
	auto hash_fn = Botan::HashFunction::create("SHA-256");
	assert(hash_fn != nullptr);
	hash_fn->update(data);
	hash_fn->final(hash.data());
	return hash;
}

bool verify(const Signature &signature, const std::string &message, const Botan::Public_Key &key);
bool verify_certificate(const Certificate &cert, const std::string &message, const Keystore &keystore);

Signature sign(const std::string &message, const Botan::Private_Key &key);

} // namespace Quasar::Crypto
