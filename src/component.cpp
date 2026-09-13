#include "pcbtech/component.hpp"

namespace pcbtech {

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
  if (c.max_voltage_v <= 0.0) result.errors.emplace_back("maximum voltage must be positive");
  if (c.max_current_a <= 0.0) result.errors.emplace_back("maximum current must be positive");
  if (c.max_junction_c <= 0.0) result.errors.emplace_back("maximum junction temperature must be positive");

  const bool has_authoritative_source = !c.sources.empty() &&
      !c.sources.front().title.empty() && !c.sources.front().url.empty() &&
      !c.sources.front().revision.empty();

  if (c.requested_tier == AccuracyTier::Verified && !has_authoritative_source) {
    result.errors.emplace_back("verified components require a revisioned authoritative source");
    result.effective_tier = AccuracyTier::Partial;
  }
  if (c.requested_tier == AccuracyTier::Verified && !c.has_simulation_model) {
    result.errors.emplace_back("verified components require a simulation model");
    result.effective_tier = AccuracyTier::Partial;
  }
  return result;
}

}  // namespace pcbtech

