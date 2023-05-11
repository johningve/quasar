#include <cassert>
#include <utility>

#include "crypto.h"
#include "exception.h"

namespace Quasar::Crypto
{

Signature sign(const std::string &message, const Botan::Private_Key &private_key)
{
	// EMSA1 does not add any padding. Deprecated in Botan version 3.
	auto signer = Botan::PK_Signer(private_key, Botan::system_rng(), "EMSA1(SHA-256)");
	signer.update(message);
	auto signature = signer.signature(Botan::system_rng());
	assert(signature.size() == SIGNATURE_LENGTH);

	std::array<uint8_t, SIGNATURE_LENGTH> signature_arr{};
	std::copy(signature.begin(), signature.begin() + SIGNATURE_LENGTH, signature_arr.begin());

	return Signature{signature_arr, (Identity)hash(private_key.public_key_bits())};
}

bool verify(const Signature &signature, const std::string &message, const Botan::Public_Key &key)
{
	auto verifier = Botan::PK_Verifier(key, "EMSA1(SHA-256)");
	verifier.update(message);
	return verifier.check_signature(signature.data().begin(), signature.data().size());
}

bool verify_certificate(const Certificate &cert, const std::string &message, const Keystore &keystore)
{
	return std::ranges::all_of(cert.signatures(), [&](auto &sig) {
		auto key = keystore.find_public_key(sig.signer());
		if (key == nullptr)
		{
			throw QUASAR_EXCEPTION_KIND(Exception::Kind::NOT_FOUND, "public key for identity {:.8} not found",
			                            sig.signer().to_hex_string());
		}
		return verify(sig, message, *key);
	});
}

} // namespace Quasar::Crypto