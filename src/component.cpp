#include "formfactor/component.hpp"

#include <algorithm>
#include <cmath>
#include <string_view>

namespace formfactor {
namespace {

bool valid_source_text(std::string_view text) {
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

bool ascii_alphanumeric(char character) {
  return (character >= 'a' && character <= 'z') ||
         (character >= '0' && character <= '9');
}

bool valid_dns_host(std::string_view authority) {
  if (authority.empty() || authority.find('@') != std::string_view::npos) {
    return false;
  }

  const auto colon = authority.find(':');
  auto host = authority;
  if (colon != std::string_view::npos) {
    if (authority.find(':', colon + 1) != std::string_view::npos) return false;
    host = authority.substr(0, colon);
    const auto port = authority.substr(colon + 1);
    if (port.empty() ||
        !std::all_of(port.begin(), port.end(), [](const char character) {
          return character >= '0' && character <= '9';
        })) {
      return false;
    }
  }
  if (host.empty()) return false;

  std::size_t label_start = 0;
  while (label_start < host.size()) {
    const auto dot = host.find('.', label_start);
    const auto label_end = dot == std::string_view::npos ? host.size() : dot;
    const auto label = host.substr(label_start, label_end - label_start);
    if (label.empty() || !ascii_alphanumeric(label.front()) ||
        !ascii_alphanumeric(label.back()) ||
        !std::all_of(label.begin(), label.end(), [](const char character) {
          return ascii_alphanumeric(character) || character == '-';
        })) {
      return false;
    }
    if (dot == std::string_view::npos) return true;
    label_start = dot + 1;
  }
  return false;
}

}  // namespace

bool valid_source_url(std::string_view url) {
  constexpr std::string_view prefix{"https://"};
  if (!url.starts_with(prefix) || url.find('#') != std::string_view::npos) {
    return false;
  }
  for (const unsigned char character : url) {
    if (character <= 0x20U || character == 0x7fU) return false;
  }

  const auto remainder = url.substr(prefix.size());
  const auto authority_end = remainder.find_first_of("/?");
  const auto authority = remainder.substr(0, authority_end);
  return valid_dns_host(authority);
}

bool valid_source(const Source& source) {
  return valid_source_text(source.title) && valid_source_url(source.url) &&
         valid_source_text(source.revision);
}

bool ValidationResult::export_allowed() const {
  return errors.empty() && effective_tier == AccuracyTier::Verified;
}

ValidationResult validate(const Component& c) {
  ValidationResult result;
  result.effective_tier = c.requested_tier;

  if (c.manufacturer.empty()) result.errors.emplace_back("manufacturer is required");
  if (c.part_number.empty()) result.errors.emplace_back("part number is required");
  if (c.package.empty()) result.errors.emplace_back("package is required");
  if (c.pin_count == 0) result.errors.emplace_back("pin count must be positive");
  // Rejecting only values <= 0 lets NaN and positive infinity through.
  // Preserve the existing positive-rating contract, but require finite inputs.
  if (!std::isfinite(c.max_voltage_v) || c.max_voltage_v <= 0.0) {
    result.errors.emplace_back("maximum voltage must be finite and positive (V)");
  }
  if (!std::isfinite(c.max_current_a) || c.max_current_a <= 0.0) {
    result.errors.emplace_back("maximum current must be finite and positive (A)");
  }
  if (!std::isfinite(c.max_junction_c) || c.max_junction_c <= 0.0) {
    result.errors.emplace_back(
        "maximum junction temperature must be finite and positive (deg C)");
  }

  const bool has_authoritative_source =
      !c.sources.empty() && valid_source(c.sources.front());

  if (c.requested_tier == AccuracyTier::Verified && !has_authoritative_source) {
    result.errors.emplace_back("verified components require a revisioned authoritative HTTPS source");
    result.effective_tier = AccuracyTier::Partial;
  }
  if (c.requested_tier == AccuracyTier::Verified && !c.has_simulation_model) {
    result.errors.emplace_back("verified components require a simulation model");
    result.effective_tier = AccuracyTier::Partial;
  }
  return result;
}

}  // namespace formfactor
