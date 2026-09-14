#include "formfactor/catalog.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <string>

namespace {

formfactor::Source source() {
  return {"Synthetic CR-007 fixture", "https://example.invalid/cr-007", "1"};
}

formfactor::AssetLink asset(const char* identifier) {
  return {identifier, "1", source()};
}

formfactor::CatalogEntry entry(formfactor::ComponentFamily family) {
  const auto value =
      formfactor::make_quantity(5, 0, formfactor::Unit::Volt);
  assert(value.has_value());
  return {
      "fixture:cr-007",
      "Synthetic family applicability fixture",
      family,
      {},
      {},
      {"Fixture", "CR-007", "SOT-23", 2, 5.0, 1.0, 125.0, true,
       formfactor::AccuracyTier::Verified, {source()}},
      asset("symbol"),
      asset("footprint"),
      asset("physical-model"),
      asset("simulation-model"),
      {{"1", "1", "p"}, {"2", "2", "n"}},
      {{"voltage.maximum", "Maximum voltage", *value,
        formfactor::QuantityValueQualifier::Maximum, std::nullopt,
        "Fixture", "CR-007", source(), std::string(64, 'a')}}};
}

bool contains(const std::vector<formfactor::ComponentFamily>& families,
              formfactor::ComponentFamily family) {
  return std::find(families.begin(), families.end(), family) != families.end();
}

}  // namespace

int main() {
  const auto first =
      formfactor::catalog_quantity_property_schema("voltage.maximum");
  const auto replay =
      formfactor::catalog_quantity_property_schema("voltage.maximum");
  assert(first.has_value());
  assert(replay.has_value());
  assert(first->applicable_families == replay->applicable_families);
  assert(!first->applicable_families.empty());
  assert(contains(first->applicable_families,
                  formfactor::ComponentFamily::Protection));
  assert(!contains(first->applicable_families,
                   formfactor::ComponentFamily::Other));

  const auto supported =
      formfactor::validate_catalog_entry(entry(formfactor::ComponentFamily::Protection));
  assert(supported.quantity_properties_export_allowed());

  const auto unsupported =
      formfactor::validate_catalog_entry(entry(formfactor::ComponentFamily::Other));
  assert(!unsupported.linked_model_ready());
  assert(!unsupported.quantity_properties_export_allowed());
  assert(unsupported.canonical_quantity_property_record.empty());
  assert(unsupported.errors ==
         std::vector<std::string>{
             "quantity property voltage.maximum is unsupported for the component family"});
}
