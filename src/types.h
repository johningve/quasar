#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "quasar.pb.h"
#include "util/array_hasher.h"

namespace Quasar
{

using std::byte;
using std::uint64_t;
using std::uint8_t;

const int HASH_LENGTH = 32;
const int SIGNATURE_LENGTH = 32;

typedef uint64_t Round;

typedef std::array<uint8_t, HASH_LENGTH> Hash;
typedef Hash Identity;

class Signature
{
  public:
	Signature(const std::array<uint8_t, SIGNATURE_LENGTH> &data, const Identity &signer);
	const std::array<uint8_t, SIGNATURE_LENGTH> &data() const;
	const Identity &signer() const;

	Proto::Signature to_proto() const;

  private:
	std::array<uint8_t, SIGNATURE_LENGTH> m_data;
	Identity m_signer;
};

class Certificate
{
  public:
	explicit Certificate(const std::vector<Signature> &signatures);
	const std::vector<Signature> &signatures() const;

	Proto::Certificate to_proto() const;

  private:
	std::vector<Signature> m_signatures;
};

class Block
{
  public:
	Block(const Hash &m_parent, Certificate m_certificate, Round m_round, const std::vector<byte> &m_payload);

	Hash hash() const;
	const Hash &parent() const;
	const Certificate &certificate() const;
	Round round() const;
	const std::vector<byte> &payload() const;

	Proto::Block to_proto() const;

  private:
	Hash m_hash;
	Hash m_parent;
	Certificate m_certificate;
	Round m_round;
	std::vector<byte> m_payload;
};

const Certificate GENESIS_CERT{{}};

const Block GENESIS{Hash{}, GENESIS_CERT, 0, {}};

} // namespace Quasar