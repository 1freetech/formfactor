#include "formfactor/catalog.hpp"

#include <cassert>
#include <string>

namespace {
formfactor::Source source() {
  return {"Synthetic fixture", "https://example.invalid/source", "Rev A"};
}

formfactor::CatalogEntry entry() {
  const auto value = formfactor::make_quantity(5, 0, formfactor::Unit::Volt);
  assert(value.has_value());
  const formfactor::Component component{
      "Vendor", "PART-2", "DFN-2", 2, 5.0, 1.0, 125.0, true,
      formfactor::AccuracyTier::Verified, {source()}};
  return {"vendor:part-2:dfn-2", "Fixture", formfactor::ComponentFamily::Protection,
          {}, {}, component,
          {"symbol", "1", source()}, {"footprint", "1", source()},
          {"model", "1", source()}, {"simulation", "1", source()},
          {{"1", "1", "p"}, {"2", "2", "n"}},
          {{"voltage.maximum", "Maximum voltage", *value,
            formfactor::QuantityValueQualifier::Maximum, std::nullopt,
            "Vendor", "PART-2", source(),
            "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"}}};
}
}  // namespace

int main() {
  const auto valid = entry();
  const auto first = formfactor::validate_catalog_entry(valid);
  const auto replay = formfactor::validate_catalog_entry(valid);
  assert(first.quantity_properties_export_allowed());
  assert(first.canonical_quantity_property_record == replay.canonical_quantity_property_record);
  assert(first.canonical_quantity_property_record.find(
             "source-artifact-sha256=64:0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\n") !=
         std::string::npos);

  for (const std::string& digest : {
           std::string{}, std::string(63, 'a'), std::string(65, 'a'),
           std::string(64, 'A'), std::string(63, 'a') + "g"}) {
    auto invalid = valid;
    invalid.quantity_properties.front().source_artifact_sha256 = digest;
    const auto result = formfactor::validate_catalog_entry(invalid);
    assert(!result.quantity_properties_export_allowed());
    assert(result.canonical_quantity_property_record.empty());
    assert(result.errors == formfactor::validate_catalog_entry(invalid).errors);
  }

  auto boundary = valid;
  boundary.quantity_properties.front().source_artifact_sha256 = std::string(64, 'f');
  assert(formfactor::validate_catalog_entry(boundary).quantity_properties_export_allowed());
}
