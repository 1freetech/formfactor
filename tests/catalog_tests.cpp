#include "formfactor/catalog.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <locale>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace {

class GroupedDigits final : public std::numpunct<char> {
 protected:
  char do_thousands_sep() const override { return '_'; }
  std::string do_grouping() const override { return "\1"; }
};

formfactor::AssetLink asset(const char* id) {
  return {id,
          "1.0",
          {"Authoritative library record", "https://example.invalid/asset", "1.0"}};
}

formfactor::Source property_source() {
  return {"Synthetic datasheet fixture",
          "https://example.invalid/property", "Rev A"};
}

formfactor::Quantity quantity(std::int64_t coefficient, int decimal_exponent,
                           formfactor::Unit unit) {
  const auto value =
      formfactor::make_quantity(coefficient, decimal_exponent, unit);
  assert(value.has_value());
  return *value;
}

formfactor::CatalogQuantityProperty property(
    const char* id, const char* name, formfactor::Quantity value,
    formfactor::QuantityValueQualifier qualifier,
    std::optional<std::string> conditions = std::nullopt) {
  return {id, name, value, qualifier, std::move(conditions), property_source()};
}

}  // namespace

int main() {
  formfactor::Component component{
      "Vendor", "PART-2", "DFN-2", 2, 16.0, 2.0, 150.0, true,
      formfactor::AccuracyTier::Verified,
      {{"Datasheet", "https://vendor.invalid/part-2", "Rev A"}}};

  formfactor::CatalogEntry entry{
      "vendor:part-2:dfn-2",
      "Example two-terminal device",
      formfactor::ComponentFamily::Protection,
      {"two-terminal", "protection"},
      {"example device"},
      component,
      asset("symbols/vendor/part-2"),
      asset("footprints/vendor/dfn-2"),
      asset("3d/vendor/dfn-2.step"),
      asset("models/vendor/part-2.lib"),
      {{"1", "1", "p"}, {"2", "2", "n"}},
      {property("voltage.maximum", "Maximum voltage",
                quantity(16, 0, formfactor::Unit::Volt),
                formfactor::QuantityValueQualifier::Maximum),
       property("capacitance.nominal", "Nominal capacitance",
                quantity(1, 0, formfactor::Unit::Microfarad),
                formfactor::QuantityValueQualifier::Nominal,
                "at stated test conditions")}};

  const auto valid = formfactor::validate_catalog_entry(entry);
  assert(valid.linked_model_ready());
  assert(valid.quantity_properties_export_allowed());
  assert(valid.canonical_quantity_property_record ==
         formfactor::validate_catalog_entry(entry)
             .canonical_quantity_property_record);
  assert(valid.canonical_quantity_property_record.find(
             "formfactor-catalog-quantity-properties-v1\n") == 0);
  assert(valid.canonical_quantity_property_record.find("property-count=2\n") !=
         std::string::npos);
  assert(valid.canonical_quantity_property_record.find(
             "quantity=capacitance:1e-6\n") != std::string::npos);
  assert(valid.canonical_quantity_property_record.find(
             "quantity=voltage:16e0\n") != std::string::npos);

  // Canonical record integers never inherit the process display locale.
  const std::locale original_locale = std::locale();
  std::locale::global(std::locale{original_locale, new GroupedDigits});
  const auto localized_record =
      formfactor::validate_catalog_entry(entry).canonical_quantity_property_record;
  std::locale::global(original_locale);
  assert(localized_record == valid.canonical_quantity_property_record);

  // Input order cannot change the replay record.
  auto reversed = entry;
  std::reverse(reversed.quantity_properties.begin(),
               reversed.quantity_properties.end());
  assert(formfactor::validate_catalog_entry(reversed)
             .canonical_quantity_property_record ==
         valid.canonical_quantity_property_record);

  // Equivalent input units produce the same exact SI catalogue record.
  auto one_volt = entry;
  one_volt.quantity_properties = {
      property("voltage.nominal", "Nominal voltage",
               quantity(1, 0, formfactor::Unit::Volt),
               formfactor::QuantityValueQualifier::Nominal)};
  auto thousand_millivolts = one_volt;
  thousand_millivolts.quantity_properties.front().value =
      quantity(1000, 0, formfactor::Unit::Millivolt);
  assert(formfactor::validate_catalog_entry(one_volt)
             .canonical_quantity_property_record ==
         formfactor::validate_catalog_entry(thousand_millivolts)
             .canonical_quantity_property_record);

  // Missing property values stay absent: no value is manufactured from the
  // legacy component ratings or the component family.
  auto no_properties = entry;
  no_properties.quantity_properties.clear();
  const auto missing = formfactor::validate_catalog_entry(no_properties);
  assert(missing.linked_model_ready());
  assert(missing.quantity_properties_export_allowed());
  assert(missing.canonical_quantity_property_record.find(
             "property-count=0\n") != std::string::npos);
  assert(missing.canonical_quantity_property_record.find("quantity=") ==
         std::string::npos);

  // Absence and presence of an opaque source condition are distinct states.
  auto conditioned = one_volt;
  conditioned.quantity_properties.front().conditions =
      "at stated test conditions";
  const auto unconditional_record = formfactor::validate_catalog_entry(one_volt)
                                        .canonical_quantity_property_record;
  const auto conditioned_record = formfactor::validate_catalog_entry(conditioned)
                                      .canonical_quantity_property_record;
  assert(unconditional_record.find("conditions-present=0\n") !=
         std::string::npos);
  assert(conditioned_record.find("conditions-present=1\n") !=
         std::string::npos);
  assert(unconditional_record != conditioned_record);

  // Every supported qualifier is stable; unknown enum values fail closed.
  const std::array qualifier_cases{
      std::pair{formfactor::QuantityValueQualifier::Nominal, "nominal"},
      std::pair{formfactor::QuantityValueQualifier::Minimum, "minimum"},
      std::pair{formfactor::QuantityValueQualifier::Typical, "typical"},
      std::pair{formfactor::QuantityValueQualifier::Maximum, "maximum"}};
  for (const auto& [qualifier, name] : qualifier_cases) {
    auto qualified = one_volt;
    qualified.quantity_properties.front().qualifier = qualifier;
    const auto result = formfactor::validate_catalog_entry(qualified);
    assert(result.quantity_properties_export_allowed());
    assert(result.canonical_quantity_property_record.find(
               std::string{"qualifier="} + name + "\n") !=
           std::string::npos);
  }

  const auto expect_rejected = [](const formfactor::CatalogEntry& invalid) {
    const auto first = formfactor::validate_catalog_entry(invalid);
    const auto replay = formfactor::validate_catalog_entry(invalid);
    assert(!first.linked_model_ready());
    assert(!first.quantity_properties_export_allowed());
    assert(first.canonical_quantity_property_record.empty());
    assert(first.errors == replay.errors);
  };

  for (const int raw : {-1, 4, std::numeric_limits<int>::max()}) {
    auto invalid = one_volt;
    invalid.quantity_properties.front().qualifier =
        static_cast<formfactor::QuantityValueQualifier>(raw);
    expect_rejected(invalid);
  }

  // Property IDs are stable machine identifiers, not free-form labels.
  const std::array<const char*, 7> invalid_ids{
      "", "Voltage", "voltage/value", "voltage value", "voltage.",
      ".voltage", "voltage\nforged"};
  for (const char* id : invalid_ids) {
    auto invalid = one_volt;
    invalid.quantity_properties.front().property_id = id;
    expect_rejected(invalid);
  }

  auto duplicate_property = entry;
  duplicate_property.quantity_properties[1].property_id =
      duplicate_property.quantity_properties[0].property_id;
  expect_rejected(duplicate_property);

  for (const char* display_name : {"", "   ", "Voltage\nforged"}) {
    auto invalid = one_volt;
    invalid.quantity_properties.front().display_name = display_name;
    expect_rejected(invalid);
  }

  for (const char* conditions : {"", "   ", "condition\nforged"}) {
    auto invalid = one_volt;
    invalid.quantity_properties.front().conditions = conditions;
    expect_rejected(invalid);
  }

  for (const std::size_t source_field : {std::size_t{0}, std::size_t{1},
                                         std::size_t{2}}) {
    for (const char* invalid_text : {"", "   ", "source\nforged"}) {
      auto invalid = one_volt;
      auto& source = invalid.quantity_properties.front().source;
      std::array<std::string*, 3> fields{
          &source.title, &source.url, &source.revision};
      *fields[source_field] = invalid_text;
      expect_rejected(invalid);
    }
  }

  auto forged_catalog_id = one_volt;
  forged_catalog_id.catalog_id = "vendor:part\nproperty-count=99";
  expect_rejected(forged_catalog_id);

  // The largest supported exact decimal coefficient/exponent survives the
  // catalogue boundary without floating-point conversion or truncation.
  const auto boundary_quantity = formfactor::make_si_quantity(
      std::numeric_limits<std::int64_t>::max(), 30,
      formfactor::Dimension::Power);
  assert(boundary_quantity.has_value());
  auto boundary = entry;
  boundary.quantity_properties = {
      property("power.maximum", "Maximum power", *boundary_quantity,
               formfactor::QuantityValueQualifier::Maximum)};
  const auto boundary_result = formfactor::validate_catalog_entry(boundary);
  assert(boundary_result.quantity_properties_export_allowed());
  assert(boundary_result.canonical_quantity_property_record.find(
             "quantity=power:9223372036854775807e30\n") !=
         std::string::npos);

  auto duplicate_pad = entry;
  duplicate_pad.pin_mappings[1].footprint_pad = "1";
  assert(!formfactor::validate_catalog_entry(duplicate_pad).linked_model_ready());

  auto missing_physical_model = entry;
  missing_physical_model.physical_model.identifier.clear();
  assert(!formfactor::validate_catalog_entry(missing_physical_model).linked_model_ready());

  auto incomplete_pin_mapping = entry;
  incomplete_pin_mapping.pin_mappings.pop_back();
  assert(!formfactor::validate_catalog_entry(incomplete_pin_mapping).linked_model_ready());

  // A package may expose more than one physical pad for one logical terminal.
  auto repeated_logical_pin = entry;
  repeated_logical_pin.pin_mappings.insert(
      repeated_logical_pin.pin_mappings.begin() + 1, {"1", "EP", "p"});
  assert(formfactor::validate_catalog_entry(repeated_logical_pin).linked_model_ready());

  // Linked assets must not conceal an invalid component rating. These are
  // synthetic numeric fixtures, not manufacturer-verified parts or assets.
  const std::array rating_fields{
      &formfactor::Component::max_voltage_v,
      &formfactor::Component::max_current_a,
      &formfactor::Component::max_junction_c};
  const std::array non_finite_values{
      std::numeric_limits<double>::quiet_NaN(),
      std::numeric_limits<double>::infinity(),
      -std::numeric_limits<double>::infinity()};
  const std::array tiers{formfactor::AccuracyTier::Verified,
                        formfactor::AccuracyTier::Partial,
                        formfactor::AccuracyTier::VisualOnly};
  for (const auto tier : tiers) {
    for (const auto field : rating_fields) {
      for (const auto value : non_finite_values) {
        auto invalid = entry;
        invalid.component.requested_tier = tier;
        invalid.component.*field = value;
        const auto result = formfactor::validate_catalog_entry(invalid);
        assert(!result.linked_model_ready());
        assert(!result.quantity_properties_export_allowed());
        assert(result.canonical_quantity_property_record.empty());
        const auto component_result = formfactor::validate(invalid.component);
        assert(component_result.errors.size() == 1);
        assert(result.errors == std::vector<std::string>{
                                    "component: " + component_result.errors.front()});
        assert(formfactor::validate_catalog_entry(invalid).errors == result.errors);
      }
    }
  }
}
