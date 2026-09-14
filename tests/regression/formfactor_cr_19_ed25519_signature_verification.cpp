#include "formfactor/hash.hpp"
#include "formfactor/policy_signature.hpp"

#include <cassert>
#include <string>
#include <string_view>

namespace {

unsigned char nibble(char character) {
  if (character >= '0' && character <= '9') {
    return static_cast<unsigned char>(character - '0');
  }
  if (character >= 'a' && character <= 'f') {
    return static_cast<unsigned char>(character - 'a' + 10);
  }
  assert(false);
  return 0;
}

std::string hex_bytes(std::string_view hex) {
  assert(hex.size() % 2U == 0U);
  std::string bytes;
  bytes.reserve(hex.size() / 2U);
  for (std::size_t index = 0; index < hex.size(); index += 2U) {
    bytes.push_back(static_cast<char>(
        (nibble(hex[index]) << 4U) | nibble(hex[index + 1U])));
  }
  return bytes;
}

formfactor::DetachedPolicySignatureEvidence rfc8032_test_1() {
  const auto public_key = hex_bytes(
      "d75a980182b10ab7d54bfed3c964073a"
      "0ee172f3daa62325af021a68f707511a");
  const auto signature = hex_bytes(
      "e5564300c360ac729086e2cc806e828a"
      "84877f1eb8e5d974d873e06522490155"
      "5fb8821590a33bacc61e39701cf9b46b"
      "d25bf5f0595bbe24655141438e7a100b");
  return {{formfactor::PolicySigningAlgorithm::Ed25519,
           "rfc8032-test-1", public_key,
           formfactor::sha256_hex(public_key).value()},
          std::string{}, signature};
}

}  // namespace

int main() {
  const auto vector = rfc8032_test_1();
  const auto first =
      formfactor::verify_detached_ed25519_signature(vector);
  const auto replay =
      formfactor::verify_detached_ed25519_signature(vector);
  assert(first.verified());
  assert(first.decision ==
         formfactor::DetachedSignatureVerificationDecision::Verified);
  assert(first.errors.empty());
  assert(first.backend_name == "OpenSSL EVP Ed25519");
  assert(!first.backend_version.empty());
  assert(first.canonical_record == replay.canonical_record);
  assert(first.canonical_record.find(
             "formfactor-detached-signature-verification-v1\n") == 0);
  assert(first.canonical_record.find("decision=verified\n") !=
         std::string::npos);

  auto changed_signature = vector;
  (*changed_signature.signature_bytes)[0] ^= 0x01;
  const auto rejected =
      formfactor::verify_detached_ed25519_signature(changed_signature);
  assert(!rejected.verified());
  assert(rejected.decision ==
         formfactor::DetachedSignatureVerificationDecision::Rejected);
  assert(rejected.errors.empty());
  assert(!rejected.canonical_record.empty());
  assert(rejected.canonical_record.find("decision=rejected\n") !=
         std::string::npos);
  assert(rejected.canonical_record != first.canonical_record);

  auto changed_message = vector;
  changed_message.signed_bytes = std::string{"x"};
  const auto wrong_message =
      formfactor::verify_detached_ed25519_signature(changed_message);
  assert(wrong_message.decision ==
         formfactor::DetachedSignatureVerificationDecision::Rejected);
  assert(!wrong_message.verified());

  auto malformed = vector;
  malformed.signature_bytes = std::string(63, '\0');
  const auto invalid =
      formfactor::verify_detached_ed25519_signature(malformed);
  assert(invalid.decision ==
         formfactor::DetachedSignatureVerificationDecision::Invalid);
  assert(!invalid.verified());
  assert(!invalid.errors.empty());
  assert(invalid.backend_name.empty());
  assert(invalid.backend_version.empty());
  assert(invalid.canonical_record.empty());

  auto wrong_fingerprint = vector;
  wrong_fingerprint.signing_key.public_key_sha256 = std::string(64, '0');
  const auto invalid_key =
      formfactor::verify_detached_ed25519_signature(wrong_fingerprint);
  assert(invalid_key.decision ==
         formfactor::DetachedSignatureVerificationDecision::Invalid);
  assert(!invalid_key.verified());
  assert(!invalid_key.errors.empty());
  assert(invalid_key.canonical_record.empty());
}
