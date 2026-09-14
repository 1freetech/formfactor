#include "formfactor/hash.hpp"
#include "formfactor/source_authority.hpp"

#include <cassert>
#include <string>

namespace {

constexpr char abc_digest[] =
    "ba7816bf8f01cfea414140de5dae2223"
    "b00361a396177a9cb410ff61f20015ad";
constexpr char empty_digest[] =
    "e3b0c44298fc1c149afbf4c8996fb924"
    "27ae41e4649b934ca495991b7852b855";

formfactor::SourcePublisherClaim claim() {
  return {"fixture.publisher",
          {"Synthetic source claim", "https://fixture.invalid/data",
           "Revision 16"}};
}

formfactor::PublisherAuthorizationPolicy policy() {
  return {"fixture.publisher",
          {"https://fixture.invalid"},
          {"Synthetic authorization policy",
           "https://authority.invalid/policies/fixture", "Revision 16"},
          abc_digest,
          std::string{"abc"}};
}

void expect_invalid(const formfactor::PublisherAuthorizationPolicy& candidate) {
  const auto first = formfactor::authorize_source_publisher(claim(), candidate);
  const auto replay = formfactor::authorize_source_publisher(claim(), candidate);
  assert(first.decision == formfactor::SourceAuthorizationDecision::Invalid);
  assert(!first.authorized());
  assert(!first.errors.empty());
  assert(first.canonical_record.empty());
  assert(first.errors == replay.errors);
}

}  // namespace

int main() {
  assert(formfactor::sha256_hex("").value() == empty_digest);
  assert(formfactor::sha256_hex("abc").value() == abc_digest);
  assert(formfactor::sha256_hex(
             "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq")
             .value() ==
         "248d6a61d20638b8e5c026930c3e6039"
         "a33ce45964ff2167f6ecedd419db06c1");
  assert(formfactor::valid_sha256_hex(abc_digest));
  assert(!formfactor::valid_sha256_hex(std::string(64, 'A')));

  const auto valid = formfactor::authorize_source_publisher(claim(), policy());
  assert(valid.authorized());
  assert(valid.errors.empty());
  assert(valid.canonical_record.find(
             "formfactor-source-authorization-v3\n") == 0);
  assert(valid.canonical_record.find("policy-artifact-byte-count=3\n") !=
         std::string::npos);

  auto missing_bytes = policy();
  missing_bytes.artifact_bytes.reset();
  expect_invalid(missing_bytes);

  auto modified_bytes = policy();
  modified_bytes.artifact_bytes = std::string{"abd"};
  expect_invalid(modified_bytes);

  auto mismatched_digest = policy();
  mismatched_digest.artifact_sha256 = empty_digest;
  expect_invalid(mismatched_digest);

  formfactor::PublisherAuthorizationPolicy empty_artifact{
      "x",
      {"https://x"},
      {"x", "https://x", "x"},
      empty_digest,
      std::string{}};
  const auto boundary = formfactor::authorize_source_publisher(
      {"x", {"x", "https://x", "x"}}, empty_artifact);
  assert(boundary.authorized());
  assert(boundary.canonical_record.find("policy-artifact-byte-count=0\n") !=
         std::string::npos);

  std::string binary_bytes{"a\0b", 3};
  auto binary_policy = policy();
  binary_policy.artifact_bytes = binary_bytes;
  binary_policy.artifact_sha256 =
      formfactor::sha256_hex(binary_bytes).value();
  const auto binary_result =
      formfactor::authorize_source_publisher(claim(), binary_policy);
  assert(binary_result.authorized());
  assert(binary_result.canonical_record.find("policy-artifact-byte-count=3\n") !=
         std::string::npos);
}
