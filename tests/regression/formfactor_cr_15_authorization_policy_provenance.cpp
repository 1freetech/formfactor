#include "formfactor/source_authority.hpp"

#include <cassert>
#include <string>

namespace {

formfactor::SourcePublisherClaim claim(std::string url =
                                          "https://fixture.invalid/data") {
  return {"fixture.publisher",
          {"Synthetic source claim", url, "Revision 15"}};
}

formfactor::PublisherAuthorizationPolicy policy() {
  return {"fixture.publisher",
          {"https://fixture.invalid"},
          {"Synthetic authorization policy",
           "https://authority.invalid/policies/fixture", "Revision 15"},
          std::string(64, 'a')};
}

void expect_invalid(const formfactor::PublisherAuthorizationPolicy& candidate) {
  const auto first = formfactor::authorize_source_publisher(claim(), candidate);
  const auto replay = formfactor::authorize_source_publisher(claim(), candidate);
  assert(first.decision == formfactor::SourceAuthorizationDecision::Invalid);
  assert(!first.authorized());
  assert(!first.errors.empty());
  assert(first.canonical_record.empty());
  assert(first.decision == replay.decision);
  assert(first.errors == replay.errors);
  assert(first.canonical_record == replay.canonical_record);
}

}  // namespace

int main() {
  const auto valid = formfactor::authorize_source_publisher(claim(), policy());
  assert(valid.authorized());
  assert(valid.errors.empty());
  assert(valid.canonical_record.find(
             "formfactor-source-authorization-v2\n") == 0);
  assert(valid.canonical_record.find(
             "policy-source-url=42:https://authority.invalid/policies/fixture\n") !=
         std::string::npos);
  assert(valid.canonical_record.find(
             "policy-artifact-sha256=64:aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
             "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") != std::string::npos);

  const auto replay = formfactor::authorize_source_publisher(claim(), policy());
  assert(replay.canonical_record == valid.canonical_record);

  const auto denied = formfactor::authorize_source_publisher(
      claim("https://other.invalid/data"), policy());
  assert(denied.decision ==
         formfactor::SourceAuthorizationDecision::NotAuthorized);
  assert(!denied.authorized());
  assert(denied.errors.empty());
  assert(!denied.canonical_record.empty());
  assert(denied.canonical_record.find("policy-artifact-sha256=64:") !=
         std::string::npos);

  auto missing_source = policy();
  missing_source.source = {};
  expect_invalid(missing_source);

  auto insecure_source = policy();
  insecure_source.source.url = "http://authority.invalid/policy";
  expect_invalid(insecure_source);

  auto missing_digest = policy();
  missing_digest.artifact_sha256.clear();
  expect_invalid(missing_digest);

  auto short_digest = policy();
  short_digest.artifact_sha256 = std::string(63, 'a');
  expect_invalid(short_digest);

  auto long_digest = policy();
  long_digest.artifact_sha256 = std::string(65, 'a');
  expect_invalid(long_digest);

  auto uppercase_digest = policy();
  uppercase_digest.artifact_sha256 = std::string(64, 'A');
  expect_invalid(uppercase_digest);

  auto non_hex_digest = policy();
  non_hex_digest.artifact_sha256 = std::string(64, 'g');
  expect_invalid(non_hex_digest);

  formfactor::PublisherAuthorizationPolicy boundary{
      "x", {"https://x"}, {"x", "https://x", "x"}, std::string(64, '0')};
  const auto boundary_result = formfactor::authorize_source_publisher(
      {"x", {"x", "https://x", "x"}}, boundary);
  assert(boundary_result.authorized());
}
