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

}  // namespace formfactor
