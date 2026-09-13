#include "pcbtech/component.hpp"
#include <cassert>

int main() {
  pcbtech::Component verified{
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
}

