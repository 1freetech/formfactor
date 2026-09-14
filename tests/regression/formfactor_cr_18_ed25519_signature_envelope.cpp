#include "formfactor/hash.hpp"
#include "formfactor/policy_trust.hpp"

#include <cassert>
#include <string>

namespace {

formfactor::DetachedPolicySignatureEvidence evidence() {
  std::string public_key(32, '\x01');
  std::string signed_bytes{"policy\nbytes\0end", 16};
  return {{formfactor::PolicySigningAlgorithm::Ed25519,
           "fixture-policy-key-1", public_key,
           formfactor::sha256_hex(public_key).value()},
          signed_bytes, std::string(64, '\x02')};
}

void expect_invalid(
    const formfactor::DetachedPolicySignatureEvidence& candidate) {
  const auto first =
      formfactor::validate_detached_policy_signature_evidence(candidate);
  const auto replay =
      formfactor::validate_detached_policy_signature_evidence(candidate);
  assert(!first.evidence_complete());
  assert(!first.errors.empty());
  assert(first.canonical_record.empty());
  assert(first.errors == replay.errors);
}

}  // namespace

int main() {
  const auto candidate = evidence();
  const auto first =
      formfactor::validate_detached_policy_signature_evidence(candidate);
  const auto replay =
      formfactor::validate_detached_policy_signature_evidence(candidate);
  assert(first.evidence_complete());
  assert(first.errors.empty());
  assert(first.canonical_record == replay.canonical_record);
  assert(first.canonical_record.find(
             "formfactor-detached-signature-evidence-v1\n") == 0);
  assert(first.canonical_record.find("algorithm=7:ed25519\n") !=
         std::string::npos);
  assert(first.canonical_record.find("signed-byte-count=16\n") !=
         std::string::npos);
  assert(first.canonical_record.find("signature-byte-count=64\n") !=
         std::string::npos);
  assert(first.canonical_record.find(
             "cryptographic-verification=not-performed\n") !=
         std::string::npos);
  assert(first.canonical_record.find("policy\nbytes") == std::string::npos);

  auto invalid_key = candidate;
  invalid_key.signing_key.public_key_bytes = std::string(31, '\x01');
  expect_invalid(invalid_key);

  auto missing_signed_bytes = candidate;
  missing_signed_bytes.signed_bytes.reset();
  expect_invalid(missing_signed_bytes);

  auto missing_signature = candidate;
  missing_signature.signature_bytes.reset();
  expect_invalid(missing_signature);

  auto short_signature = candidate;
  short_signature.signature_bytes = std::string(63, '\x02');
  expect_invalid(short_signature);

  auto long_signature = candidate;
  long_signature.signature_bytes = std::string(65, '\x02');
  expect_invalid(long_signature);

  auto empty_message = candidate;
  empty_message.signed_bytes = std::string{};
  const auto empty_result =
      formfactor::validate_detached_policy_signature_evidence(empty_message);
  assert(empty_result.evidence_complete());
  assert(empty_result.canonical_record.find("signed-byte-count=0\n") !=
         std::string::npos);
  assert(empty_result.canonical_record.find(
             "e3b0c44298fc1c149afbf4c8996fb924"
             "27ae41e4649b934ca495991b7852b855") != std::string::npos);

  auto changed_message = candidate;
  changed_message.signed_bytes = std::string{"different bytes"};
  const auto changed_message_result =
      formfactor::validate_detached_policy_signature_evidence(changed_message);
  assert(changed_message_result.evidence_complete());
  assert(changed_message_result.canonical_record != first.canonical_record);

  auto changed_signature = candidate;
  changed_signature.signature_bytes = std::string(64, '\x03');
  const auto changed_signature_result =
      formfactor::validate_detached_policy_signature_evidence(changed_signature);
  assert(changed_signature_result.evidence_complete());
  assert(changed_signature_result.canonical_record != first.canonical_record);
}
