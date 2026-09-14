#include "formfactor/catalog_filter.hpp"

#include <cassert>
#include <limits>

namespace {

formfactor::Source source() {
  return {"Synthetic fixture", "https://example.invalid/source", "Rev A"};
}

formfactor::AssetLink asset(const char* id) { return {id, "1", source()}; }

formfactor::Quantity quantity(std::int64_t coefficient, formfactor::Unit unit) {
  const auto value = formfactor::make_quantity(coefficient, 0, unit);
  assert(value.has_value());
  return *value;
}

formfactor::CatalogEntry entry() {
  return {"fixture:resistor", "Synthetic resistor",
          formfactor::ComponentFamily::Passive, {}, {},
          {"Fixture", "R-1", "0603", 2, 50.0, 0.1, 125.0, true,
           formfactor::AccuracyTier::Verified, {source()}},
          asset("symbol"), asset("footprint"), asset("physical"),
          asset("simulation"), {{"1", "1", "p"}, {"2", "2", "n"}},
          {{"resistance.nominal", "Nominal resistance",
            quantity(1000, formfactor::Unit::Ohm),
            formfactor::QuantityValueQualifier::Nominal, std::nullopt,
            source()},
           {"power.maximum", "Maximum power",
            quantity(250, formfactor::Unit::Milliwatt),
            formfactor::QuantityValueQualifier::Maximum,
            "at 70 degrees C ambient", source()}}};
}

}  // namespace

int main() {
  using formfactor::CatalogFilterOutcome;
  using formfactor::CatalogQuantityFilter;
  using formfactor::NumericRelation;
  using formfactor::QuantityValueQualifier;

  const auto catalog = entry();
  const CatalogQuantityFilter equal{
      "resistance.nominal", QuantityValueQualifier::Nominal,
      NumericRelation::Equal, quantity(1, formfactor::Unit::Kiloohm),
      std::nullopt};
  const auto match = formfactor::evaluate_catalog_quantity_filter(catalog, equal);
  assert(match.matched());
  assert(match.outcome == CatalogFilterOutcome::Match);
  assert(match.canonical_record ==
         formfactor::evaluate_catalog_quantity_filter(catalog, equal)
             .canonical_record);

  auto boundary = equal;
  boundary.relation = NumericRelation::AtLeast;
  assert(formfactor::evaluate_catalog_quantity_filter(catalog, boundary).matched());
  boundary.relation = NumericRelation::AtMost;
  assert(formfactor::evaluate_catalog_quantity_filter(catalog, boundary).matched());

  auto no_match = equal;
  no_match.threshold = quantity(999, formfactor::Unit::Ohm);
  assert(formfactor::evaluate_catalog_quantity_filter(catalog, no_match).outcome ==
         CatalogFilterOutcome::NoMatch);

  auto missing = equal;
  missing.property_id = "inductance.nominal";
  const auto unknown =
      formfactor::evaluate_catalog_quantity_filter(catalog, missing);
  assert(unknown.outcome == CatalogFilterOutcome::Unknown);
  assert(!unknown.matched());

  auto wrong_qualifier = equal;
  wrong_qualifier.qualifier = QuantityValueQualifier::Maximum;
  assert(formfactor::evaluate_catalog_quantity_filter(catalog, wrong_qualifier)
             .outcome == CatalogFilterOutcome::Unknown);

  CatalogQuantityFilter conditioned{
      "power.maximum", QuantityValueQualifier::Maximum,
      NumericRelation::AtLeast, quantity(250, formfactor::Unit::Milliwatt),
      std::nullopt};
  assert(formfactor::evaluate_catalog_quantity_filter(catalog, conditioned)
             .outcome == CatalogFilterOutcome::Unknown);
  conditioned.conditions = "at 70 degrees C ambient";
  assert(formfactor::evaluate_catalog_quantity_filter(catalog, conditioned)
             .matched());

  auto wrong_dimension = equal;
  wrong_dimension.threshold = quantity(1, formfactor::Unit::Volt);
  const auto invalid_dimension =
      formfactor::evaluate_catalog_quantity_filter(catalog, wrong_dimension);
  assert(invalid_dimension.outcome == CatalogFilterOutcome::Invalid);
  assert(!invalid_dimension.errors.empty());
  assert(invalid_dimension.canonical_record.empty());

  auto invalid_relation = equal;
  invalid_relation.relation = static_cast<NumericRelation>(
      std::numeric_limits<int>::max());
  assert(formfactor::evaluate_catalog_quantity_filter(catalog, invalid_relation)
             .outcome == CatalogFilterOutcome::Invalid);

  auto invalid_qualifier = equal;
  invalid_qualifier.qualifier = static_cast<QuantityValueQualifier>(99);
  assert(formfactor::evaluate_catalog_quantity_filter(catalog,
                                                       invalid_qualifier)
             .outcome == CatalogFilterOutcome::Invalid);

  auto invalid_id = equal;
  invalid_id.property_id = "Resistance\nforged";
  assert(formfactor::evaluate_catalog_quantity_filter(catalog, invalid_id)
             .outcome == CatalogFilterOutcome::Invalid);

  auto invalid_catalog = catalog;
  invalid_catalog.quantity_properties.front().source.revision.clear();
  const auto blocked =
      formfactor::evaluate_catalog_quantity_filter(invalid_catalog, equal);
  assert(blocked.outcome == CatalogFilterOutcome::Invalid);
  assert(!blocked.matched());
  assert(blocked.canonical_record.empty());
}
