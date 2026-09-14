#pragma once

#include <optional>
#include <string>
#include <vector>

namespace formfactor {

// RFC 8032 fixes an Ed25519 public key at exactly 32 octets. This type records
// caller-supplied key material; validation does not establish ownership, trust,
// signature validity, revocation status, or policy authority.
enum class PolicySigningAlgorithm { Ed25519 };

struct PolicySigningKey {
  PolicySigningAlgorithm algorithm{PolicySigningAlgorithm::Ed25519};
  std::string key_id;
  std::optional<std::string> public_key_bytes;
  std::string public_key_sha256;
};

struct PolicySigningKeyValidationResult {
  std::vector<std::string> errors;
  std::string canonical_record;

  [[nodiscard]] bool fingerprint_verified() const;
};

[[nodiscard]] PolicySigningKeyValidationResult validate_policy_signing_key(
    const PolicySigningKey& key);

// This envelope preserves exact detached-signature inputs for future
// cryptographic verification. Structural completeness is not signature
// validity and does not establish key ownership or policy authority.
struct DetachedPolicySignatureEvidence {
  PolicySigningKey signing_key;
  std::optional<std::string> signed_bytes;
  std::optional<std::string> signature_bytes;
};

struct DetachedPolicySignatureEvidenceResult {
  std::vector<std::string> errors;
  std::string canonical_record;

  [[nodiscard]] bool evidence_complete() const;
};

[[nodiscard]] DetachedPolicySignatureEvidenceResult
validate_detached_policy_signature_evidence(
    const DetachedPolicySignatureEvidence& evidence);

}  // namespace formfactor
