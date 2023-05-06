#include <utility>

#include "crypto.h"
#include "types.h"

namespace Quasar
{

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

const std::vector<Signature> &Certificate::signatures() const
{
	return m_signatures;
}

Certificate::Certificate(const std::vector<Signature> &signatures) : m_signatures(signatures)
{
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

const std::vector<byte> &Block::payload() const
{
	return m_payload;
}

Block::Block(const Hash &m_parent, Certificate m_certificate, Round m_round, const std::vector<byte> &m_payload)
    : m_parent(m_parent), m_certificate(std::move(m_certificate)), m_round(m_round), m_payload(m_payload)
{
	m_hash = Crypto::hash(to_proto().SerializeAsString());
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
	proto.set_payload(m_payload.data(), m_payload.size());
	auto cert_ptr = proto.mutable_certificate();
	*cert_ptr = m_certificate.to_proto();
	return proto;
}
} // namespace Quasar