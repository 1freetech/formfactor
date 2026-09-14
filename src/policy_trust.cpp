#include "formfactor/policy_trust.hpp"

#include "formfactor/hash.hpp"

#include <string_view>

namespace formfactor {
namespace {

bool valid_record_text(std::string_view text) {
  bool visible = false;
  for (const unsigned char character : text) {
    if (character <= 0x20U || character == 0x7fU) {
      if (character != ' ') return false;
    } else {
      visible = true;
    }
  }
  return visible;
}

std::string_view algorithm_name(PolicySigningAlgorithm algorithm) {
  switch (algorithm) {
    case PolicySigningAlgorithm::Ed25519:
      return "ed25519";
  }
  return {};
}

void append_field(std::string& record, std::string_view name,
                  std::string_view value) {
  record.append(name);
  record.push_back('=');
  record.append(std::to_string(value.size()));
  record.push_back(':');
  record.append(value);
  record.push_back('\n');
}

}  // namespace

bool PolicySigningKeyValidationResult::fingerprint_verified() const {
  return errors.empty() && !canonical_record.empty();
}

PolicySigningKeyValidationResult validate_policy_signing_key(
    const PolicySigningKey& key) {
  PolicySigningKeyValidationResult result;
  const auto algorithm = algorithm_name(key.algorithm);

  if (algorithm.empty()) {
    result.errors.emplace_back("policy signing algorithm is unsupported");
  }
  if (!valid_record_text(key.key_id)) {
    result.errors.emplace_back(
        "policy signing key ID must be visible single-line text");
  }
  if (!key.public_key_bytes.has_value()) {
    result.errors.emplace_back("policy signing public-key bytes are required");
  } else if (key.public_key_bytes->size() != 32U) {
    result.errors.emplace_back(
        "Ed25519 public key must contain exactly 32 bytes");
  }

  const bool digest_is_valid = valid_sha256_hex(key.public_key_sha256);
  if (!digest_is_valid) {
    result.errors.emplace_back(
        "policy signing key requires a lowercase 64-hex SHA-256 fingerprint");
  }
  if (key.public_key_bytes.has_value() &&
      key.public_key_bytes->size() == 32U && digest_is_valid) {
    const auto calculated = sha256_hex(*key.public_key_bytes);
    if (!calculated || *calculated != key.public_key_sha256) {
      result.errors.emplace_back(
          "policy signing key fingerprint does not match supplied bytes");
    }
  }

  if (!result.errors.empty()) return result;

  result.canonical_record = "formfactor-policy-signing-key-v1\n";
  append_field(result.canonical_record, "algorithm", algorithm);
  append_field(result.canonical_record, "key-id", key.key_id);
  result.canonical_record.append("public-key-byte-count=32\n");
  append_field(result.canonical_record, "public-key-sha256",
               key.public_key_sha256);
  return result;
}

}  // namespace formfactor
