#include "pcbtech/catalog.hpp"

#include <cassert>

namespace {

pcbtech::AssetLink asset(const char* id) {
  return {id,
          "1.0",
          {"Authoritative library record", "https://example.invalid/asset", "1.0"}};
}

}  // namespace

int main() {
  pcbtech::Component component{
      "Vendor", "PART-2", "DFN-2", 2, 30.0, 2.0, 150.0, true,
      pcbtech::AccuracyTier::Verified,
      {{"Datasheet", "https://vendor.invalid/part-2", "Rev A"}}};

  pcbtech::CatalogEntry entry{
      "vendor:part-2:dfn-2",
      "Example two-terminal device",
      pcbtech::ComponentFamily::Protection,
      {"two-terminal", "protection"},
      {"example device"},
      component,
      asset("symbols/vendor/part-2"),
      asset("footprints/vendor/dfn-2"),
      asset("3d/vendor/dfn-2.step"),
      asset("models/vendor/part-2.lib"),
      {{"1", "1", "p"}, {"2", "2", "n"}}};

  assert(pcbtech::validate_catalog_entry(entry).linked_model_ready());

  auto duplicate_pad = entry;
  duplicate_pad.pin_mappings[1].footprint_pad = "1";
  assert(!pcbtech::validate_catalog_entry(duplicate_pad).linked_model_ready());

  auto missing_physical_model = entry;
  missing_physical_model.physical_model.identifier.clear();
  assert(!pcbtech::validate_catalog_entry(missing_physical_model).linked_model_ready());

  auto incomplete_pin_mapping = entry;
  incomplete_pin_mapping.pin_mappings.pop_back();
  assert(!pcbtech::validate_catalog_entry(incomplete_pin_mapping).linked_model_ready());

  // A package may expose more than one physical pad for one logical terminal.
  auto repeated_logical_pin = entry;
  repeated_logical_pin.pin_mappings.insert(
      repeated_logical_pin.pin_mappings.begin() + 1, {"1", "EP", "p"});
  assert(pcbtech::validate_catalog_entry(repeated_logical_pin).linked_model_ready());
}
