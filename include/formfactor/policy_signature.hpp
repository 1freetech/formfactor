#pragma once

#include "formfactor/policy_trust.hpp"

#include <string>
#include <vector>

namespace formfactor {

enum class DetachedSignatureVerificationDecision {
  Verified,
  Rejected,
  Invalid,
  BackendError
};

struct DetachedSignatureVerificationResult {
  DetachedSignatureVerificationDecision decision{
      DetachedSignatureVerificationDecision::Invalid};
  std::string backend_name;
  std::string backend_version;
  std::vector<std::string> errors;
  std::string canonical_record;

  [[nodiscard]] bool verified() const;
};

// Performs one-shot PureEdDSA verification over the exact supplied bytes using
// OpenSSL EVP. A verified signature proves possession of the private key
// corresponding to the supplied public key, but not that the key is authorized.
[[nodiscard]] DetachedSignatureVerificationResult
verify_detached_ed25519_signature(
    const DetachedPolicySignatureEvidence& evidence);

}  // namespace formfactor
