#pragma once

#include "formfactor/component.hpp"

#include <optional>
#include <string>
#include <vector>

namespace formfactor {

enum class SourceAuthorizationDecision { Authorized, NotAuthorized, Invalid };

struct PublisherAuthorizationPolicy {
  std::string publisher_id;
  std::vector<std::string> authorized_origins;
  // Provenance for the exact policy artifact supplied by the caller. These
  // fields bind replay evidence; they do not authenticate the publisher.
  Source source;
  std::string artifact_sha256;
  // Exact caller-supplied bytes. Absence is unknown; an empty string is a
  // valid, distinct artifact whose standard SHA-256 digest can be verified.
  std::optional<std::string> artifact_bytes;
};

struct SourcePublisherClaim {
  std::string publisher_id;
  Source source;
};

struct SourceAuthorizationResult {
  SourceAuthorizationDecision decision{SourceAuthorizationDecision::Invalid};
  std::vector<std::string> errors;
  std::string canonical_record;

  [[nodiscard]] bool authorized() const;
};

[[nodiscard]] SourceAuthorizationResult authorize_source_publisher(
    const SourcePublisherClaim& claim,
    const PublisherAuthorizationPolicy& policy);

}  // namespace formfactor
