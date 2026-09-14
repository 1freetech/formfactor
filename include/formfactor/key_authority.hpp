#pragma once

#include "formfactor/policy_trust.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace formfactor {

struct TrustedPolicyKeyBinding {
  std::string publisher_id;
  PolicySigningKey signing_key;
};

struct PolicyKeyTrustRoot {
  std::string root_id;
  std::uint64_t version{0};
  std::string expires_utc;
  std::vector<TrustedPolicyKeyBinding> trusted_keys;
};

enum class PolicyKeyTrustDecision { Trusted, NotTrusted, Expired, Invalid };

struct PolicyKeyTrustResult {
  PolicyKeyTrustDecision decision{PolicyKeyTrustDecision::Invalid};
  std::vector<std::string> errors;
  std::string canonical_record;

  [[nodiscard]] bool trusted() const;
};

// Uses TUF's fixed YYYY-MM-DDTHH:MM:SSZ UTC timestamp profile. This evaluates
// an explicitly supplied root; it does not authenticate, persist, or
// rollback-protect that root.
[[nodiscard]] PolicyKeyTrustResult evaluate_policy_key_trust(
    const PolicyKeyTrustRoot& root, const std::string& publisher_id,
    const PolicySigningKey& candidate, const std::string& evaluation_utc);

enum class PolicyKeyRootTransitionDecision {
  Sequential,
  Rollback,
  VersionGap,
  DifferentRoot,
  Expired,
  Invalid
};

struct PolicyKeyRootTransitionResult {
  PolicyKeyRootTransitionDecision decision{
      PolicyKeyRootTransitionDecision::Invalid};
  std::vector<std::string> errors;
  std::string canonical_record;

  [[nodiscard]] bool sequence_valid() const;
};

// Checks the TUF N-to-N+1 version and final-expiry prerequisites only.
// Signature thresholds, persistence, and trust-root authenticity are not
// evaluated here.
[[nodiscard]] PolicyKeyRootTransitionResult
validate_policy_key_root_transition(
    const PolicyKeyTrustRoot& current_root,
    const PolicyKeyTrustRoot& candidate_root,
    const std::string& evaluation_utc);

}  // namespace formfactor
