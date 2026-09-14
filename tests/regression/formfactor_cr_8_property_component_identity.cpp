#include "formfactor/catalog.hpp"

#include <cassert>
#include <string>

namespace {

formfactor::Source source() {
  return {"Synthetic source", "https://example.invalid/cr-008", "Rev A"};
}

formfactor::AssetLink asset(const char* id) { return {id, "1", source()}; }

formfactor::CatalogEntry entry() {
  const auto voltage = formfactor::make_quantity(5, 0, formfactor::Unit::Volt);
  assert(voltage.has_value());
  return {
      "fixture:cr-008", "Synthetic identity fixture",
      formfactor::ComponentFamily::Protection, {}, {},
      {"Fixture Maker", "FF-008", "SOT-23", 2, 5.0, 1.0, 125.0, true,
       formfactor::AccuracyTier::Verified, {source()}},
      asset("symbol"), asset("footprint"), asset("physical"), asset("model"),
      {{"1", "1", "p"}, {"2", "2", "n"}},
      {{"voltage.maximum", "Maximum voltage", *voltage,
        formfactor::QuantityValueQualifier::Maximum, std::nullopt,
        "Fixture Maker", "FF-008", source(), std::string(64, 'a'), "table 1"}}};
}

}  // namespace

int main() {
  const auto valid = formfactor::validate_catalog_entry(entry());
  assert(valid.quantity_properties_export_allowed());
  assert(valid.canonical_quantity_property_record ==
         formfactor::validate_catalog_entry(entry())
             .canonical_quantity_property_record);
  assert(valid.canonical_quantity_property_record.find(
             "claimed-manufacturer=13:Fixture Maker\n") != std::string::npos);
  assert(valid.canonical_quantity_property_record.find(
             "claimed-part-number=6:FF-008\n") != std::string::npos);

  for (const bool manufacturer : {false, true}) {
    auto invalid = entry();
    auto& property = invalid.quantity_properties.front();
    (manufacturer ? property.claimed_manufacturer
                  : property.claimed_part_number) = "different";
    const auto result = formfactor::validate_catalog_entry(invalid);
    assert(!result.quantity_properties_export_allowed());
    assert(result.canonical_quantity_property_record.empty());
    assert(result.errors == formfactor::validate_catalog_entry(invalid).errors);

    auto forged = entry();
    auto& forged_property = forged.quantity_properties.front();
    (manufacturer ? forged_property.claimed_manufacturer
                  : forged_property.claimed_part_number) = "value\nforged";
    assert(!formfactor::validate_catalog_entry(forged)
                .quantity_properties_export_allowed());
  }
}
