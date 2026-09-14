#include "formfactor/source_authority.hpp"

#include <algorithm>
#include <string_view>

namespace formfactor {
namespace {

bool valid_record_text(std::string_view text) {
  bool visible = false;
  for (const unsigned char character : text) {
    if (character <= 0x20U || character == 0x7fU) {
      if (character != ' ') return false;
    } else {
      visible = true;
    }
  }
  return visible;
}

void append_field(std::string& record, std::string_view name,
                  std::string_view value) {
  record.append(name);
  record.push_back('=');
  record.append(std::to_string(value.size()));
  record.push_back(':');
  record.append(value);
  record.push_back('\n');
}

}  // namespace

bool SourceAuthorizationResult::authorized() const {
  return decision == SourceAuthorizationDecision::Authorized && errors.empty() &&
         !canonical_record.empty();
}

SourceAuthorizationResult authorize_source_publisher(
    const SourcePublisherClaim& claim,
    const PublisherAuthorizationPolicy& policy) {
  SourceAuthorizationResult result;

  if (!valid_record_text(claim.publisher_id)) {
    result.errors.emplace_back("source publisher ID must be visible single-line text");
  }
  if (!valid_source(claim.source)) {
    result.errors.emplace_back("source metadata is invalid");
  }
  if (!valid_record_text(policy.publisher_id)) {
    result.errors.emplace_back("policy publisher ID must be visible single-line text");
  }
  if (policy.authorized_origins.empty()) {
    result.errors.emplace_back("policy requires at least one authorized HTTPS origin");
  }

  auto origins = policy.authorized_origins;
  for (const auto& origin : origins) {
    const auto parsed = source_url_origin(origin);
    if (!parsed || *parsed != origin) {
      result.errors.emplace_back(
          "policy origin must be an exact canonical HTTPS origin");
    }
  }
  std::sort(origins.begin(), origins.end());
  if (std::adjacent_find(origins.begin(), origins.end()) != origins.end()) {
    result.errors.emplace_back("policy contains a duplicate authorized origin");
  }

  if (!result.errors.empty()) return result;

  const auto source_origin = source_url_origin(claim.source.url);
  if (!source_origin) {
    result.errors.emplace_back("source URL has no canonical HTTPS origin");
    return result;
  }

  const bool authorized =
      claim.publisher_id == policy.publisher_id &&
      std::binary_search(origins.begin(), origins.end(), *source_origin);
  result.decision = authorized ? SourceAuthorizationDecision::Authorized
                               : SourceAuthorizationDecision::NotAuthorized;

  result.canonical_record = "formfactor-source-authorization-v1\n";
  append_field(result.canonical_record, "claim-publisher", claim.publisher_id);
  append_field(result.canonical_record, "policy-publisher", policy.publisher_id);
  append_field(result.canonical_record, "source-url", claim.source.url);
  append_field(result.canonical_record, "source-origin", *source_origin);
  result.canonical_record.append("authorized-origin-count=");
  result.canonical_record.append(std::to_string(origins.size()));
  result.canonical_record.push_back('\n');
  for (const auto& origin : origins) {
    append_field(result.canonical_record, "authorized-origin", origin);
  }
  result.canonical_record.append(authorized ? "decision=authorized\n"
                                            : "decision=not-authorized\n");
  return result;
}

}  // namespace formfactor
