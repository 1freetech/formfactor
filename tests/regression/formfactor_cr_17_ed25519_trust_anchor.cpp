#include "formfactor/hash.hpp"
#include "formfactor/policy_trust.hpp"

#include <cassert>
#include <string>

namespace {

formfactor::PolicySigningKey key() {
  std::string bytes(32, '\x01');
  return {formfactor::PolicySigningAlgorithm::Ed25519,
          "fixture-policy-key-1", bytes,
          formfactor::sha256_hex(bytes).value()};
}

void expect_invalid(const formfactor::PolicySigningKey& candidate) {
  const auto first = formfactor::validate_policy_signing_key(candidate);
  const auto replay = formfactor::validate_policy_signing_key(candidate);
  assert(!first.fingerprint_verified());
  assert(!first.errors.empty());
  assert(first.canonical_record.empty());
  assert(first.errors == replay.errors);
}

}  // namespace

int main() {
  const auto candidate = key();
  const auto first = formfactor::validate_policy_signing_key(candidate);
  const auto replay = formfactor::validate_policy_signing_key(candidate);
  assert(first.fingerprint_verified());
  assert(first.errors.empty());
  assert(first.canonical_record == replay.canonical_record);
  assert(first.canonical_record.find(
             "formfactor-policy-signing-key-v1\n") == 0);
  assert(first.canonical_record.find("algorithm=7:ed25519\n") !=
         std::string::npos);
  assert(first.canonical_record.find("key-id=20:fixture-policy-key-1\n") !=
         std::string::npos);
  assert(first.canonical_record.find("public-key-byte-count=32\n") !=
         std::string::npos);

  auto missing_bytes = candidate;
  missing_bytes.public_key_bytes.reset();
  expect_invalid(missing_bytes);

  auto short_key = candidate;
  short_key.public_key_bytes = std::string(31, '\x01');
  short_key.public_key_sha256 =
      formfactor::sha256_hex(*short_key.public_key_bytes).value();
  expect_invalid(short_key);

  auto long_key = candidate;
  long_key.public_key_bytes = std::string(33, '\x01');
  long_key.public_key_sha256 =
      formfactor::sha256_hex(*long_key.public_key_bytes).value();
  expect_invalid(long_key);

  auto modified_key = candidate;
  modified_key.public_key_bytes = std::string(32, '\x02');
  expect_invalid(modified_key);

  auto missing_fingerprint = candidate;
  missing_fingerprint.public_key_sha256.clear();
  expect_invalid(missing_fingerprint);

  auto uppercase_fingerprint = candidate;
  uppercase_fingerprint.public_key_sha256 = std::string(64, 'A');
  expect_invalid(uppercase_fingerprint);

  auto non_hex_fingerprint = candidate;
  non_hex_fingerprint.public_key_sha256 = std::string(64, 'g');
  expect_invalid(non_hex_fingerprint);

  auto short_fingerprint = candidate;
  short_fingerprint.public_key_sha256 = std::string(63, '0');
  expect_invalid(short_fingerprint);

  auto empty_id = candidate;
  empty_id.key_id.clear();
  expect_invalid(empty_id);

  auto control_id = candidate;
  control_id.key_id = "fixture\nkey";
  expect_invalid(control_id);

  auto unsupported_algorithm = candidate;
  unsupported_algorithm.algorithm =
      static_cast<formfactor::PolicySigningAlgorithm>(99);
  expect_invalid(unsupported_algorithm);

  std::string zero_bytes(32, '\0');
  formfactor::PolicySigningKey binary{
      formfactor::PolicySigningAlgorithm::Ed25519, "binary-key", zero_bytes,
      formfactor::sha256_hex(zero_bytes).value()};
  const auto binary_result =
      formfactor::validate_policy_signing_key(binary);
  assert(binary_result.fingerprint_verified());
  assert(binary_result.canonical_record != first.canonical_record);
}
