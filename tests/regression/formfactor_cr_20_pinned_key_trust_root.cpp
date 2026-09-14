#include "formfactor/hash.hpp"
#include "formfactor/key_authority.hpp"

#include <algorithm>
#include <cassert>
#include <string>
#include <utility>

namespace {

formfactor::PolicySigningKey key(std::string id, char byte) {
  std::string bytes(32, byte);
  return {formfactor::PolicySigningAlgorithm::Ed25519, std::move(id), bytes,
          formfactor::sha256_hex(bytes).value()};
}

formfactor::PolicyKeyTrustRoot root() {
  return {"fixture-root", 7, "2027-01-01T00:00:00Z",
          {{"publisher-b", key("key-b", '\x02')},
           {"publisher-a", key("key-a", '\x01')}}};
}

void expect_invalid(const formfactor::PolicyKeyTrustRoot& candidate_root,
                    const std::string& publisher,
                    const formfactor::PolicySigningKey& candidate_key,
                    const std::string& evaluated_at) {
  const auto first = formfactor::evaluate_policy_key_trust(
      candidate_root, publisher, candidate_key, evaluated_at);
  const auto replay = formfactor::evaluate_policy_key_trust(
      candidate_root, publisher, candidate_key, evaluated_at);
  assert(first.decision == formfactor::PolicyKeyTrustDecision::Invalid);
  assert(!first.trusted());
  assert(!first.errors.empty());
  assert(first.canonical_record.empty());
  assert(first.errors == replay.errors);
}

}  // namespace

int main() {
  const auto policy = root();
  const auto candidate = key("key-a", '\x01');
  const auto first = formfactor::evaluate_policy_key_trust(
      policy, "publisher-a", candidate, "2026-12-31T23:59:59Z");
  const auto replay = formfactor::evaluate_policy_key_trust(
      policy, "publisher-a", candidate, "2026-12-31T23:59:59Z");
  assert(first.trusted());
  assert(first.decision == formfactor::PolicyKeyTrustDecision::Trusted);
  assert(first.errors.empty());
  assert(first.canonical_record == replay.canonical_record);
  assert(first.canonical_record.find(
             "formfactor-policy-key-trust-v1\n") == 0);
  assert(first.canonical_record.find("root-version=7\n") !=
         std::string::npos);
  assert(first.canonical_record.find("decision=trusted\n") !=
         std::string::npos);
  assert(first.canonical_record.find(std::string(32, '\x01')) ==
         std::string::npos);

  auto reordered = policy;
  std::reverse(reordered.trusted_keys.begin(), reordered.trusted_keys.end());
  const auto reordered_result = formfactor::evaluate_policy_key_trust(
      reordered, "publisher-a", candidate, "2026-12-31T23:59:59Z");
  assert(reordered_result.canonical_record == first.canonical_record);

  const auto expired = formfactor::evaluate_policy_key_trust(
      policy, "publisher-a", candidate, "2027-01-01T00:00:00Z");
  assert(expired.decision == formfactor::PolicyKeyTrustDecision::Expired);
  assert(!expired.trusted());
  assert(expired.errors.empty());
  assert(expired.canonical_record.find("decision=expired\n") !=
         std::string::npos);

  const auto wrong_publisher = formfactor::evaluate_policy_key_trust(
      policy, "publisher-b", candidate, "2026-01-01T00:00:00Z");
  assert(wrong_publisher.decision ==
         formfactor::PolicyKeyTrustDecision::NotTrusted);

  const auto unknown_key = formfactor::evaluate_policy_key_trust(
      policy, "publisher-a", key("key-c", '\x03'),
      "2026-01-01T00:00:00Z");
  assert(unknown_key.decision ==
         formfactor::PolicyKeyTrustDecision::NotTrusted);

  auto rotated = policy;
  rotated.version = 8;
  rotated.trusted_keys.erase(rotated.trusted_keys.begin() + 1);
  const auto removed_key = formfactor::evaluate_policy_key_trust(
      rotated, "publisher-a", candidate, "2026-01-01T00:00:00Z");
  assert(removed_key.decision ==
         formfactor::PolicyKeyTrustDecision::NotTrusted);

  auto invalid_version = policy;
  invalid_version.version = 0;
  expect_invalid(invalid_version, "publisher-a", candidate,
                 "2026-01-01T00:00:00Z");

  auto no_keys = policy;
  no_keys.trusted_keys.clear();
  expect_invalid(no_keys, "publisher-a", candidate,
                 "2026-01-01T00:00:00Z");

  auto duplicate = policy;
  duplicate.trusted_keys.push_back(
      {"publisher-a", key("key-a", '\x04')});
  expect_invalid(duplicate, "publisher-a", candidate,
                 "2026-01-01T00:00:00Z");

  auto malformed_key = policy;
  malformed_key.trusted_keys.front().signing_key.public_key_bytes =
      std::string(31, '\x02');
  expect_invalid(malformed_key, "publisher-a", candidate,
                 "2026-01-01T00:00:00Z");

  auto invalid_candidate = candidate;
  invalid_candidate.public_key_sha256 = std::string(64, '0');
  expect_invalid(policy, "publisher-a", invalid_candidate,
                 "2026-01-01T00:00:00Z");

  auto invalid_expiry = policy;
  invalid_expiry.expires_utc = "2026-02-29T00:00:00Z";
  expect_invalid(invalid_expiry, "publisher-a", candidate,
                 "2026-01-01T00:00:00Z");

  expect_invalid(policy, "publisher-a", candidate,
                 "2026-01-01T00:00:00+00:00");
  expect_invalid(policy, "publisher-a", candidate,
                 "2026-13-01T00:00:00Z");
  expect_invalid(policy, "publisher-a", candidate,
                 "2026-01-01T24:00:00Z");

  auto leap_root = policy;
  leap_root.expires_utc = "2028-02-29T00:00:00Z";
  const auto leap = formfactor::evaluate_policy_key_trust(
      leap_root, "publisher-a", candidate, "2028-02-28T23:59:59Z");
  assert(leap.trusted());
}
