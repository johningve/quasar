#include <botan/exceptn.h>
#include <botan/hex.h>
#include <iomanip>
#include <sstream>
#include <utility>

#include "crypto.h"
#include "exception.h"
#include "types.h"

namespace Quasar
{

Hash::Hash() : std::array<uint8_t, HASH_LENGTH>()
{
}

Hash Hash::from_byte_string(const std::string &byte_string)
{
	Hash hash;

	if (byte_string.size() > hash.size())
	{
		throw QUASAR_EXCEPTION("byte string with length {} exceeds hash size {}", byte_string.size(), hash.size());
	}

	std::copy(byte_string.begin(), byte_string.end(), hash.begin());

	return hash;
}

Hash Hash::from_hex_string(const std::string &hex_string)
{
	Hash hash;
	auto decoded_length = hex_string.length() / 2;
	if (decoded_length > hash.size())
	{
		throw QUASAR_EXCEPTION("hex string with decoded size {} exceeds hash size {}", decoded_length, hash.size());
	}
	try
	{
		Botan::hex_decode(hash.data(), hex_string.data(), hex_string.length());
	}
	catch (Botan::Exception &e)
	{
		throw QUASAR_EXCEPTION("{}", e.what());
	}
	return hash;
}

std::string Hash::to_byte_string() const
{
	return {begin(), end()};
}

std::string Hash::to_hex_string() const
{
	return Botan::hex_encode(data(), size());
}

Transaction::Transaction(const std::vector<byte> &data) : m_data(data)
{
	const std::vector<uint8_t> uint_vec{(const uint8_t *)m_data.data(), (const uint8_t *)m_data.data() + m_data.size()};
	m_hash = Crypto::hash(uint_vec);
}

const std::vector<byte> &Transaction::data() const
{
	return m_data;
}

const Hash &Transaction::hash() const
{
	return m_hash;
}

const std::array<uint8_t, SIGNATURE_LENGTH> &Signature::data() const
{
	return m_data;
}

const Identity &Signature::signer() const
{
	return m_signer;
}

Signature::Signature(const std::array<uint8_t, SIGNATURE_LENGTH> &data, const Identity &signer)
    : m_data(data), m_signer(signer)
{
}

Proto::Signature Signature::to_proto() const
{
	Proto::Signature proto{};
	proto.set_data(m_data.data(), m_data.size());
	proto.set_signer(m_signer.data(), m_signer.size());
	return proto;
}

Signature::Signature(const Proto::Signature &proto)
{
	std::copy_n(proto.data().begin(), std::min(proto.data().length(), SIGNATURE_LENGTH), m_data.begin());
	std::copy_n(proto.signer().begin(), std::min(proto.signer().length(), SIGNATURE_LENGTH), m_signer.begin());
}

const std::vector<Signature> &Certificate::signatures() const
{
	return m_signatures;
}

Certificate::Certificate(const std::vector<Signature> &signatures) : m_signatures(signatures)
{
}

Certificate::Certificate(const Proto::Certificate &proto)
{
	std::for_each(proto.signatures().begin(), proto.signatures().end(),
	              [this](auto sig) { this->m_signatures.push_back(Signature{sig}); });
}

Proto::Certificate Certificate::to_proto() const
{
	Proto::Certificate proto{};
	std::for_each(m_signatures.begin(), m_signatures.end(), [&proto](auto signature) {
		auto ptr = proto.add_signatures();
		*ptr = signature.to_proto();
	});
	return proto;
}

const Hash &Block::parent() const
{
	return m_parent;
}

const Certificate &Block::certificate() const
{
	return m_certificate;
}

Round Block::round() const
{
	return m_round;
}

const std::vector<Transaction> &Block::payload() const
{
	return m_payload;
}

Block::Block(const Hash &m_parent, Certificate m_certificate, Round m_round, const std::vector<Transaction> &m_payload)
    : m_parent(m_parent), m_certificate(std::move(m_certificate)), m_round(m_round), m_payload(m_payload)
{
	m_hash = Crypto::hash(to_proto().SerializeAsString());
}

Block::Block(const Proto::Block &proto)
    : m_round(proto.round()), m_certificate(proto.certificate()), m_hash(Crypto::hash(proto.SerializeAsString()))
{
	std::for_each(proto.payload().transactions().begin(), proto.payload().transactions().end(), [this](auto raw_tx) {
		this->m_payload.push_back(
		    Transaction{{(const byte *)raw_tx.data(), (const byte *)raw_tx.data() + raw_tx.size()}});
	});
	std::copy_n(proto.parent().begin(), std::min(proto.parent().length(), HASH_LENGTH), m_parent.begin());
}

Hash Block::hash() const
{
	return m_hash;
}

Proto::Block Block::to_proto() const
{
	Proto::Block proto{};
	proto.set_parent(m_parent.data(), m_parent.size());
	proto.set_round(m_round);
	auto payload_mut = proto.mutable_payload();
	std::for_each(m_payload.begin(), m_payload.end(),
	              [payload_mut](auto tx) { payload_mut->add_transactions(tx.data().data(), tx.data().size()); });
	auto cert_ptr = proto.mutable_certificate();
	*cert_ptr = m_certificate.to_proto();
	return proto;
}

} // namespace Quasar