#include "formfactor/hash.hpp"
#include "formfactor/key_authority.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>

namespace {

formfactor::PolicySigningKey key(std::string id, char byte) {
  std::string bytes(32, byte);
  return {formfactor::PolicySigningAlgorithm::Ed25519, std::move(id), bytes,
          formfactor::sha256_hex(bytes).value()};
}

formfactor::PolicyKeyTrustRoot root(std::uint64_t version,
                                    std::string expires) {
  return {"fixture-root", version, std::move(expires),
          {{"publisher-b", key("key-b", '\x02')},
           {"publisher-a", key("key-a", '\x01')}}};
}

void expect_invalid(const formfactor::PolicyKeyTrustRoot& current,
                    const formfactor::PolicyKeyTrustRoot& candidate,
                    const std::string& evaluated_at) {
  const auto result = formfactor::validate_policy_key_root_transition(
      current, candidate, evaluated_at);
  assert(result.decision ==
         formfactor::PolicyKeyRootTransitionDecision::Invalid);
  assert(!result.sequence_valid());
  assert(!result.errors.empty());
  assert(result.canonical_record.empty());
}

}  // namespace

int main() {
  auto current = root(7, "2026-01-01T00:00:00Z");
  auto candidate = root(8, "2027-01-01T00:00:00Z");
  candidate.trusted_keys.erase(candidate.trusted_keys.begin() + 1);
  candidate.trusted_keys.push_back(
      {"publisher-a", key("key-c", '\x03')});

  const auto first = formfactor::validate_policy_key_root_transition(
      current, candidate, "2026-06-01T00:00:00Z");
  const auto replay = formfactor::validate_policy_key_root_transition(
      current, candidate, "2026-06-01T00:00:00Z");
  assert(first.sequence_valid());
  assert(first.errors.empty());
  assert(first.canonical_record == replay.canonical_record);
  assert(first.canonical_record.find(
             "formfactor-policy-key-root-transition-v1\n") == 0);
  assert(first.canonical_record.find("current-version=7\n") !=
         std::string::npos);
  assert(first.canonical_record.find("candidate-version=8\n") !=
         std::string::npos);
  assert(first.canonical_record.find("decision=sequential\n") !=
         std::string::npos);
  assert(first.canonical_record.find(
             "signature-verification=not-performed\n") !=
         std::string::npos);
  assert(first.canonical_record.find("persistence=not-performed\n") !=
         std::string::npos);
  assert(first.canonical_record.find(std::string(32, '\x01')) ==
         std::string::npos);

  auto reordered_current = current;
  auto reordered_candidate = candidate;
  std::reverse(reordered_current.trusted_keys.begin(),
               reordered_current.trusted_keys.end());
  std::reverse(reordered_candidate.trusted_keys.begin(),
               reordered_candidate.trusted_keys.end());
  const auto reordered = formfactor::validate_policy_key_root_transition(
      reordered_current, reordered_candidate,
      "2026-06-01T00:00:00Z");
  assert(reordered.canonical_record == first.canonical_record);

  auto same_version = candidate;
  same_version.version = 7;
  const auto rollback = formfactor::validate_policy_key_root_transition(
      current, same_version, "2026-06-01T00:00:00Z");
  assert(rollback.decision ==
         formfactor::PolicyKeyRootTransitionDecision::Rollback);
  assert(!rollback.sequence_valid());

  auto older = candidate;
  older.version = 6;
  const auto older_result = formfactor::validate_policy_key_root_transition(
      current, older, "2026-06-01T00:00:00Z");
  assert(older_result.decision ==
         formfactor::PolicyKeyRootTransitionDecision::Rollback);

  auto skipped = candidate;
  skipped.version = 9;
  const auto gap = formfactor::validate_policy_key_root_transition(
      current, skipped, "2026-06-01T00:00:00Z");
  assert(gap.decision ==
         formfactor::PolicyKeyRootTransitionDecision::VersionGap);

  auto other_root = candidate;
  other_root.root_id = "other-root";
  const auto different = formfactor::validate_policy_key_root_transition(
      current, other_root, "2026-06-01T00:00:00Z");
  assert(different.decision ==
         formfactor::PolicyKeyRootTransitionDecision::DifferentRoot);

  auto expired = candidate;
  expired.expires_utc = "2026-06-01T00:00:00Z";
  const auto expiry = formfactor::validate_policy_key_root_transition(
      current, expired, "2026-06-01T00:00:00Z");
  assert(expiry.decision ==
         formfactor::PolicyKeyRootTransitionDecision::Expired);

  auto malformed = candidate;
  malformed.expires_utc = "2026-02-30T00:00:00Z";
  expect_invalid(current, malformed, "2026-06-01T00:00:00Z");

  auto ambiguous = candidate;
  ambiguous.trusted_keys.push_back(
      {"publisher-a", key("key-c", '\x04')});
  expect_invalid(current, ambiguous, "2026-06-01T00:00:00Z");

  auto maximum = current;
  maximum.version = std::numeric_limits<std::uint64_t>::max();
  auto wrapped = candidate;
  wrapped.version = 1;
  expect_invalid(maximum, wrapped, "2026-06-01T00:00:00Z");

  expect_invalid(current, candidate, "2026-06-01T00:00:00+00:00");
}
