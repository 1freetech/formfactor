#pragma once

#include "formfactor/component.hpp"

#include <string>
#include <vector>

namespace formfactor {

enum class SourceAuthorizationDecision { Authorized, NotAuthorized, Invalid };

struct PublisherAuthorizationPolicy {
  std::string publisher_id;
  std::vector<std::string> authorized_origins;
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
