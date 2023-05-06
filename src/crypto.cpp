#include <botan/hash.h>
#include <cassert>
#include <utility>

#include "crypto.h"

namespace Quasar
{

template <typename T> Hash hash_impl(T data)
{
	Hash hash{};
	auto hash_fn = Botan::HashFunction::create("SHA-256");
	assert(hash_fn != nullptr);
	hash_fn->update(data);
	hash_fn->final(hash.data());
	return hash;
}

Hash Crypto::hash(const std::vector<uint8_t> &data)
{
	return hash_impl<const std::vector<uint8_t> &>(data);
}

Hash Crypto::hash(const std::string &data)
{
	return hash_impl<const std::string &>(data);
}

Crypto::Crypto(Botan::ECDSA_PrivateKey key) : m_key(std::move(key))
{
}

Signature Crypto::sign(const std::vector<uint8_t> &message) const
{
	auto signer = Botan::PK_Signer(m_key, Botan::system_rng(), "SHA-256");
	signer.update(message.data(), message.size());
	auto signature = signer.signature(Botan::system_rng());
	assert(signature.size() == SIGNATURE_LENGTH);

	std::array<uint8_t, SIGNATURE_LENGTH> signature_arr{};
	std::copy(signature.begin(), signature.begin() + SIGNATURE_LENGTH, signature_arr.begin());

	return Signature{signature_arr, (Identity)hash(m_key.public_key_bits())};
}

bool Crypto::verify(const Signature &signature, const std::vector<uint8_t> &message, const Botan::Public_Key &key)
{
	auto verifier = Botan::PK_Verifier(key, "SHA-256");
	verifier.update(message);
	return verifier.check_signature(signature.data().begin(), signature.data().size());
}

} // namespace Quasar