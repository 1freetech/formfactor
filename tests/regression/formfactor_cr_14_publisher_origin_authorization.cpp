#include "formfactor/source_authority.hpp"

#include <algorithm>
#include <cassert>
#include <string>
#include <vector>

namespace {

formfactor::SourcePublisherClaim claim(std::string url =
                                          "https://fixture.invalid/data?id=14") {
  return {"fixture.publisher",
          {"Synthetic publisher document", std::move(url), "Revision 14"}};
}

formfactor::PublisherAuthorizationPolicy policy() {
  return {"fixture.publisher",
          {"https://other.invalid", "https://fixture.invalid"},
          {"Synthetic policy artifact",
           "https://authority.invalid/policies/fixture", "Revision 14"},
          std::string(64, 'a')};
}

void expect_invalid(const formfactor::SourcePublisherClaim& source_claim,
                    const formfactor::PublisherAuthorizationPolicy& source_policy) {
  const auto first =
      formfactor::authorize_source_publisher(source_claim, source_policy);
  const auto replay =
      formfactor::authorize_source_publisher(source_claim, source_policy);
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
  const auto authorized =
      formfactor::authorize_source_publisher(claim(), policy());
  assert(authorized.authorized());
  assert(authorized.errors.empty());
  assert(authorized.decision ==
         formfactor::SourceAuthorizationDecision::Authorized);
  assert(authorized.canonical_record.find(
             "formfactor-source-authorization-v2\n") == 0);
  assert(authorized.canonical_record.find("source-origin=23:https://fixture.invalid\n") !=
         std::string::npos);
  assert(authorized.canonical_record.find("decision=authorized\n") !=
         std::string::npos);

  auto reversed = policy();
  std::reverse(reversed.authorized_origins.begin(),
               reversed.authorized_origins.end());
  const auto replay =
      formfactor::authorize_source_publisher(claim(), reversed);
  assert(replay.canonical_record == authorized.canonical_record);

  auto wrong_publisher = claim();
  wrong_publisher.publisher_id = "different.publisher";
  const auto publisher_denied =
      formfactor::authorize_source_publisher(wrong_publisher, policy());
  assert(publisher_denied.decision ==
         formfactor::SourceAuthorizationDecision::NotAuthorized);
  assert(!publisher_denied.authorized());
  assert(publisher_denied.errors.empty());
  assert(!publisher_denied.canonical_record.empty());

  const auto subdomain_denied = formfactor::authorize_source_publisher(
      claim("https://sub.fixture.invalid/data"), policy());
  assert(subdomain_denied.decision ==
         formfactor::SourceAuthorizationDecision::NotAuthorized);
  assert(!subdomain_denied.authorized());

  const auto port_denied = formfactor::authorize_source_publisher(
      claim("https://fixture.invalid:443/data"), policy());
  assert(port_denied.decision ==
         formfactor::SourceAuthorizationDecision::NotAuthorized);

  const formfactor::PublisherAuthorizationPolicy boundary_policy{
      "x", {"https://x"}, {"x", "https://x", "x"}, std::string(64, '0')};
  const auto boundary =
      formfactor::authorize_source_publisher(
          {"x", {"x", "https://x", "x"}}, boundary_policy);
  assert(boundary.authorized());

  auto invalid_claim = claim();
  invalid_claim.publisher_id.clear();
  expect_invalid(invalid_claim, policy());
  expect_invalid(claim("http://fixture.invalid/data"), policy());

  auto no_origins = policy();
  no_origins.authorized_origins.clear();
  expect_invalid(claim(), no_origins);

  auto path_origin = policy();
  path_origin.authorized_origins = {"https://fixture.invalid/path"};
  expect_invalid(claim(), path_origin);

  auto query_origin = policy();
  query_origin.authorized_origins = {"https://fixture.invalid?scope=data"};
  expect_invalid(claim(), query_origin);

  auto duplicate_origins = policy();
  duplicate_origins.authorized_origins = {
      "https://fixture.invalid", "https://fixture.invalid"};
  expect_invalid(claim(), duplicate_origins);

  auto invalid_policy_id = policy();
  invalid_policy_id.publisher_id = "fixture\npublisher";
  expect_invalid(claim(), invalid_policy_id);
}
