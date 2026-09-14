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

bool DetachedPolicySignatureEvidenceResult::evidence_complete() const {
  return errors.empty() && !canonical_record.empty();
}

DetachedPolicySignatureEvidenceResult
validate_detached_policy_signature_evidence(
    const DetachedPolicySignatureEvidence& evidence) {
  DetachedPolicySignatureEvidenceResult result;

  const auto key_result = validate_policy_signing_key(evidence.signing_key);
  for (const auto& error : key_result.errors) {
    result.errors.emplace_back("policy signing key: " + error);
  }
  if (!evidence.signed_bytes.has_value()) {
    result.errors.emplace_back("exact signed bytes are required");
  }
  if (!evidence.signature_bytes.has_value()) {
    result.errors.emplace_back("detached Ed25519 signature bytes are required");
  } else if (evidence.signature_bytes->size() != 64U) {
    result.errors.emplace_back(
        "detached Ed25519 signature must contain exactly 64 bytes");
  }

  if (!result.errors.empty()) return result;

  const auto signed_digest = sha256_hex(*evidence.signed_bytes);
  const auto signature_digest = sha256_hex(*evidence.signature_bytes);
  if (!signed_digest || !signature_digest) {
    result.errors.emplace_back(
        "detached signature evidence exceeds the supported SHA-256 length");
    return result;
  }

  result.canonical_record = "formfactor-detached-signature-evidence-v1\n";
  append_field(result.canonical_record, "algorithm", "ed25519");
  append_field(result.canonical_record, "key-id", evidence.signing_key.key_id);
  append_field(result.canonical_record, "public-key-sha256",
               evidence.signing_key.public_key_sha256);
  result.canonical_record.append("signed-byte-count=");
  result.canonical_record.append(
      std::to_string(evidence.signed_bytes->size()));
  result.canonical_record.push_back('\n');
  append_field(result.canonical_record, "signed-bytes-sha256", *signed_digest);
  result.canonical_record.append("signature-byte-count=64\n");
  append_field(result.canonical_record, "signature-sha256",
               *signature_digest);
  result.canonical_record.append("cryptographic-verification=not-performed\n");
  return result;
}

}  // namespace formfactor
