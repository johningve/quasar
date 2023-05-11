#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "quasar.pb.h"

namespace Quasar
{

using std::byte;
using std::uint64_t;
using std::uint8_t;

const size_t HASH_LENGTH = 32;
const size_t SIGNATURE_LENGTH = 32;

using Round = uint64_t;

class Hash : public std::array<uint8_t, HASH_LENGTH>
{
  public:
	Hash();

	static Hash from_byte_string(const std::string &byte_string);
	static Hash from_hex_string(const std::string &hex_string);

	std::string to_byte_string() const;
	std::string to_hex_string() const;
};

using Identity = Hash;

class Transaction
{
  public:
	explicit Transaction(const std::vector<byte> &data);

	const std::vector<byte> &data() const;
	const Hash &hash() const;

  private:
	std::vector<byte> m_data;
	Hash m_hash;
};

class Signature
{
  public:
	Signature(const std::array<uint8_t, SIGNATURE_LENGTH> &data, const Identity &signer);
	explicit Signature(const Proto::Signature &proto);

	const std::array<uint8_t, SIGNATURE_LENGTH> &data() const;
	const Identity &signer() const;

	Proto::Signature to_proto() const;

  private:
	std::array<uint8_t, SIGNATURE_LENGTH> m_data{};
	Identity m_signer{};
};

class Certificate
{
  public:
	explicit Certificate(const std::vector<Signature> &signatures);
	explicit Certificate(const Proto::Certificate &proto);

	const std::vector<Signature> &signatures() const;

	Proto::Certificate to_proto() const;

  private:
	std::vector<Signature> m_signatures;
};

class Block
{
  public:
	explicit Block(const Proto::Block &proto);
	Block(const Hash &m_parent, Certificate m_certificate, Round m_round, const std::vector<Transaction> &m_payload);

	Hash hash() const;
	const Hash &parent() const;
	const Certificate &certificate() const;
	Round round() const;
	const std::vector<Transaction> &payload() const;

	Proto::Block to_proto() const;

  private:
	Hash m_hash{};
	Hash m_parent{};
	Certificate m_certificate;
	Round m_round;
	std::vector<Transaction> m_payload;
};

const Certificate GENESIS_CERT{std::vector<Signature>{}};

const Block GENESIS{Hash{}, GENESIS_CERT, 0, {}};

} // namespace Quasar

namespace std
{
template <> struct hash<Quasar::Hash>
{
	size_t operator()(const Quasar::Hash &a) const noexcept
	{
		const hash<uint8_t> hasher;
		size_t h = 0;
		for (size_t i = 0; i < Quasar::HASH_LENGTH; i++)
		{
			h = h * 31 + hasher(a[i]);
		}
		return h;
	}
};
} // namespace std