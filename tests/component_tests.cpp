#include "pcbtech/component.hpp"
#include <array>
#include <cassert>
#include <limits>
#include <string>
#include <vector>

int main() {
  // Synthetic completeness fixture; no real part or model is being certified.
  const pcbtech::Component verified{
      "Vendor", "PART-1", "QFN-16", 16, 5.5, 1.0, 150.0, true,
      pcbtech::AccuracyTier::Verified,
      {{"Datasheet", "https://vendor.invalid/part-1", "Rev A"}}};
  assert(pcbtech::validate(verified).export_allowed());

  auto missing_source = verified;
  missing_source.sources.clear();
  assert(!pcbtech::validate(missing_source).export_allowed());

  auto invalid_rating = verified;
  invalid_rating.max_voltage_v = 0.0;
  assert(!pcbtech::validate(invalid_rating).export_allowed());

  auto missing_model = verified;
  missing_model.has_simulation_model = false;
  assert(!pcbtech::validate(missing_model).export_allowed());

  // Numeric boundary fixtures, not ratings for a real manufacturer part.
  const std::array rating_fields{
      &pcbtech::Component::max_voltage_v,
      &pcbtech::Component::max_current_a,
      &pcbtech::Component::max_junction_c};
  const std::array invalid_values{
      std::numeric_limits<double>::quiet_NaN(),
      std::numeric_limits<double>::infinity(),
      -std::numeric_limits<double>::infinity(),
      0.0, -0.0, -1.0, -std::numeric_limits<double>::denorm_min()};
  const std::array tiers{pcbtech::AccuracyTier::Verified,
                        pcbtech::AccuracyTier::Partial,
                        pcbtech::AccuracyTier::VisualOnly};
  const std::array<std::string, 3> rating_errors{
      "maximum voltage must be finite and positive (V)",
      "maximum current must be finite and positive (A)",
      "maximum junction temperature must be finite and positive (deg C)"};
  for (const auto tier : tiers) {
    for (std::size_t i = 0; i < rating_fields.size(); ++i) {
      const auto field = rating_fields[i];
      for (const auto value : invalid_values) {
        auto candidate = verified;
        candidate.requested_tier = tier;
        candidate.*field = value;
        const auto result = pcbtech::validate(candidate);
        assert(!result.export_allowed());
        assert(result.errors == std::vector<std::string>{rating_errors[i]});
        const auto replay = pcbtech::validate(candidate);
        assert(replay.errors == result.errors);
        assert(replay.effective_tier == result.effective_tier);
        assert(replay.export_allowed() == result.export_allowed());
      }
    }
  }

  // Small/large finite numbers test the numeric domain only. They are not
  // recommended operating limits and do not establish physical plausibility.
  const std::array positive_values{
      std::numeric_limits<double>::denorm_min(),
      std::numeric_limits<double>::min(),
      0.125, 1.0, 150.0, std::numeric_limits<double>::max()};
  for (const auto tier : tiers) {
    for (const auto field : rating_fields) {
      for (const auto value : positive_values) {
        auto candidate = verified;
        candidate.requested_tier = tier;
        candidate.*field = value;
        const auto result = pcbtech::validate(candidate);
        assert(result.errors.empty());
        assert(result.export_allowed() == (tier == pcbtech::AccuracyTier::Verified));
        assert(candidate.*field == value);
      }
    }
  }

  auto all_invalid = verified;
  all_invalid.max_voltage_v = std::numeric_limits<double>::quiet_NaN();
  all_invalid.max_current_a = std::numeric_limits<double>::infinity();
  all_invalid.max_junction_c = -std::numeric_limits<double>::infinity();
  const auto result = pcbtech::validate(all_invalid);
  const std::vector<std::string> expected_errors(rating_errors.begin(),
                                                rating_errors.end());
  assert(result.errors == expected_errors);
  assert(!result.export_allowed());
  assert(pcbtech::validate(all_invalid).errors == expected_errors);

  // Validation reports units but never converts or repairs the supplied value.
  assert(verified.max_voltage_v == 5.5);
  assert(verified.max_current_a == 1.0);
  assert(verified.max_junction_c == 150.0);
  assert(!pcbtech::validate(pcbtech::Component{}).export_allowed());
}
