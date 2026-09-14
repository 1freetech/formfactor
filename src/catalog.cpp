#include "formfactor/catalog.hpp"

#include <algorithm>
#include <iterator>
#include <locale>
#include <sstream>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace formfactor {
namespace {


std::vector<ComponentFamily> applicable_families(Dimension dimension) {
  using Family = ComponentFamily;
  switch (dimension) {
    case Dimension::Capacitance:
      return {Family::Passive, Family::Protection,
              Family::SwitchingAmplification, Family::PowerConversion,
              Family::SensorsTiming};
    case Dimension::Current:
    case Dimension::Power:
    case Dimension::Voltage:
      return {Family::Passive, Family::Protection,
              Family::SwitchingAmplification, Family::PowerConversion,
              Family::LogicComputeMemory, Family::SensorsTiming,
              Family::ConnectionsControls, Family::IndicatorsActuators};
    case Dimension::Frequency:
    case Dimension::Time:
      return {Family::SwitchingAmplification, Family::LogicComputeMemory,
              Family::SensorsTiming};
    case Dimension::Inductance:
      return {Family::Passive, Family::PowerConversion};
    case Dimension::Length:
      return {Family::ConnectionsControls};
    case Dimension::Resistance:
      return {Family::Passive, Family::Protection,
              Family::SwitchingAmplification, Family::PowerConversion,
              Family::IndicatorsActuators};
    case Dimension::Dimensionless:
      return {};
  }
  return {};
}

bool valid_source(const Source& source) {
  return !source.title.empty() && !source.url.empty() &&
         !source.revision.empty();
}

bool valid_record_text(std::string_view text) {
  bool has_non_whitespace = false;
  for (const unsigned char character : text) {
    if (character <= 0x1fU || character == 0x7fU) return false;
    if (character != ' ') has_non_whitespace = true;
  }
  return has_non_whitespace;
}

bool valid_property_id(std::string_view identifier) {
  if (identifier.empty() || identifier.front() < 'a' ||
      identifier.front() > 'z') {
    return false;
  }
  for (const char character : identifier) {
    const bool lowercase = character >= 'a' && character <= 'z';
    const bool digit = character >= '0' && character <= '9';
    if (!lowercase && !digit && character != '.' && character != '_' &&
        character != '-') {
      return false;
    }
  }
  const char last = identifier.back();
  return (last >= 'a' && last <= 'z') || (last >= '0' && last <= '9');
}

bool valid_sha256(std::string_view digest) {
  if (digest.size() != 64U) return false;
  return std::all_of(digest.begin(), digest.end(), [](const char character) {
    return (character >= '0' && character <= '9') ||
           (character >= 'a' && character <= 'f');
  });
}

const char* qualifier_name(QuantityValueQualifier qualifier) {
  switch (qualifier) {
    case QuantityValueQualifier::Nominal: return "nominal";
    case QuantityValueQualifier::Minimum: return "minimum";
    case QuantityValueQualifier::Typical: return "typical";
    case QuantityValueQualifier::Maximum: return "maximum";
  }
  return nullptr;
}

void append_text_field(std::ostringstream& output, std::string_view name,
                       std::string_view value) {
  output << name << '=' << value.size() << ':' << value << '\n';
}

void require_asset(const AssetLink& asset, const char* name,
                   std::vector<std::string>& errors) {
  if (asset.identifier.empty()) {
    errors.emplace_back(std::string{name} + " identifier is required");
  }
  if (asset.revision.empty()) {
    errors.emplace_back(std::string{name} + " revision is required");
  }
  if (!valid_source(asset.source)) {
    errors.emplace_back(std::string{name} +
                        " requires a revisioned authoritative source");
  }
}

}  // namespace

std::optional<CatalogQuantityPropertySchema> catalog_quantity_property_schema(
    const std::string& property_id) {
  // This is deliberately a small, explicit registry. New properties require
  // a reviewed dimension and qualifier instead of being inferred from text.
  static const CatalogQuantityPropertySchema schemas[] = {
      {"capacitance.nominal", Dimension::Capacitance,
       QuantityValueQualifier::Nominal},
      {"capacitance.minimum", Dimension::Capacitance, QuantityValueQualifier::Minimum},
      {"capacitance.typical", Dimension::Capacitance, QuantityValueQualifier::Typical},
      {"capacitance.maximum", Dimension::Capacitance, QuantityValueQualifier::Maximum},
      {"current.nominal", Dimension::Current, QuantityValueQualifier::Nominal},
      {"current.minimum", Dimension::Current, QuantityValueQualifier::Minimum},
      {"current.typical", Dimension::Current, QuantityValueQualifier::Typical},
      {"current.maximum", Dimension::Current, QuantityValueQualifier::Maximum},
      {"frequency.nominal", Dimension::Frequency, QuantityValueQualifier::Nominal},
      {"frequency.minimum", Dimension::Frequency, QuantityValueQualifier::Minimum},
      {"frequency.typical", Dimension::Frequency, QuantityValueQualifier::Typical},
      {"frequency.maximum", Dimension::Frequency, QuantityValueQualifier::Maximum},
      {"inductance.nominal", Dimension::Inductance, QuantityValueQualifier::Nominal},
      {"inductance.minimum", Dimension::Inductance, QuantityValueQualifier::Minimum},
      {"inductance.typical", Dimension::Inductance, QuantityValueQualifier::Typical},
      {"inductance.maximum", Dimension::Inductance, QuantityValueQualifier::Maximum},
      {"length.nominal", Dimension::Length, QuantityValueQualifier::Nominal},
      {"length.minimum", Dimension::Length, QuantityValueQualifier::Minimum},
      {"length.typical", Dimension::Length, QuantityValueQualifier::Typical},
      {"length.maximum", Dimension::Length, QuantityValueQualifier::Maximum},
      {"power.nominal", Dimension::Power, QuantityValueQualifier::Nominal},
      {"power.minimum", Dimension::Power, QuantityValueQualifier::Minimum},
      {"power.typical", Dimension::Power, QuantityValueQualifier::Typical},
      {"power.maximum", Dimension::Power, QuantityValueQualifier::Maximum},
      {"resistance.nominal", Dimension::Resistance,
       QuantityValueQualifier::Nominal},
      {"resistance.minimum", Dimension::Resistance, QuantityValueQualifier::Minimum},
      {"resistance.typical", Dimension::Resistance, QuantityValueQualifier::Typical},
      {"resistance.maximum", Dimension::Resistance, QuantityValueQualifier::Maximum},
      {"time.nominal", Dimension::Time, QuantityValueQualifier::Nominal},
      {"time.minimum", Dimension::Time, QuantityValueQualifier::Minimum},
      {"time.typical", Dimension::Time, QuantityValueQualifier::Typical},
      {"time.maximum", Dimension::Time, QuantityValueQualifier::Maximum},
      {"voltage.maximum", Dimension::Voltage,
       QuantityValueQualifier::Maximum},
      {"voltage.minimum", Dimension::Voltage,
       QuantityValueQualifier::Minimum},
      {"voltage.nominal", Dimension::Voltage,
       QuantityValueQualifier::Nominal},
      {"voltage.typical", Dimension::Voltage,
       QuantityValueQualifier::Typical},
  };
  const auto found = std::find_if(
      std::begin(schemas), std::end(schemas), [&](const auto& schema) {
        return schema.property_id == property_id;
      });
  if (found == std::end(schemas)) return std::nullopt;
  auto schema = *found;
  schema.applicable_families = applicable_families(schema.dimension);
  return schema;
}

CatalogValidationResult validate_catalog_entry(const CatalogEntry& entry) {
  CatalogValidationResult result;

  if (!valid_record_text(entry.catalog_id)) {
    result.errors.emplace_back("catalog id must be non-empty single-line text");
  }
  if (!valid_record_text(entry.display_name)) {
    result.errors.emplace_back("display name must be non-empty single-line text");
  }

  const auto component_result = validate(entry.component);
  for (const auto& error : component_result.errors) {
    result.errors.emplace_back("component: " + error);
  }

  require_asset(entry.schematic_symbol, "schematic symbol", result.errors);
  require_asset(entry.footprint, "footprint", result.errors);
  require_asset(entry.physical_model, "physical model", result.errors);
  require_asset(entry.simulation_model, "simulation model", result.errors);

  if (entry.pin_mappings.empty()) {
    result.errors.emplace_back("at least one explicit pin mapping is required");
  }

  std::unordered_set<std::string> component_pins;
  std::unordered_set<std::string> footprint_pads;

  for (const auto& mapping : entry.pin_mappings) {
    if (mapping.component_pin.empty()) {
      result.errors.emplace_back("pin mapping component pin is required");
    } else {
      component_pins.insert(mapping.component_pin);
    }

    if (mapping.footprint_pad.empty()) {
      result.errors.emplace_back("pin mapping footprint pad is required");
    } else if (!footprint_pads.insert(mapping.footprint_pad).second) {
      result.errors.emplace_back("footprint pads must map unambiguously");
    }

    if (mapping.simulation_terminal.empty()) {
      result.errors.emplace_back("pin mapping simulation terminal is required");
    }
  }

  // Multiple physical pads may legitimately connect to one component pin, so
  // mappings may outnumber logical pins. What matters here is that every
  // logical component pin is represented and every physical pad is unique.
  if (!entry.pin_mappings.empty() &&
      component_pins.size() != entry.component.pin_count) {
    result.errors.emplace_back(
        "explicit pin mappings must cover every logical component pin");
  }

  std::unordered_set<std::string> property_ids;
  for (const auto& property : entry.quantity_properties) {
    const bool id_is_valid = valid_property_id(property.property_id);
    const std::string prefix = id_is_valid
                                   ? "quantity property " + property.property_id
                                   : "quantity property";
    if (!id_is_valid) {
      result.errors.emplace_back(
          "quantity property id must use lowercase ASCII letters, digits, '.', '_', or '-' and end with a letter or digit");
    } else if (!property_ids.insert(property.property_id).second) {
      result.errors.emplace_back("quantity property ids must be unique: " +
                                 property.property_id);
    }

    const auto schema = id_is_valid
                            ? catalog_quantity_property_schema(property.property_id)
                            : std::nullopt;
    if (!schema.has_value()) {
      result.errors.emplace_back(prefix +
                                 " has no implemented semantic schema");
    } else {
      if (property.value.dimension() != schema->dimension) {
        result.errors.emplace_back(prefix +
                                   " value has the wrong physical dimension");
      }
      if (property.qualifier != schema->qualifier) {
        result.errors.emplace_back(prefix +
                                   " qualifier conflicts with its semantic schema");
      }
      if (std::find(schema->applicable_families.begin(),
                    schema->applicable_families.end(),
                    entry.primary_family) ==
          schema->applicable_families.end()) {
        result.errors.emplace_back(prefix +
                                   " is unsupported for the component family");
      }
    }

    if (!valid_record_text(property.display_name)) {
      result.errors.emplace_back(prefix +
                                 " requires a non-empty single-line display name");
    }
    if (qualifier_name(property.qualifier) == nullptr) {
      result.errors.emplace_back(prefix + " has an unsupported qualifier");
    }
    if (property.conditions.has_value() &&
        !valid_record_text(*property.conditions)) {
      result.errors.emplace_back(prefix +
                                 " conditions must be non-empty single-line text when present");
    }
    if (!valid_record_text(property.claimed_manufacturer) ||
        !valid_record_text(property.claimed_part_number)) {
      result.errors.emplace_back(prefix +
                                 " requires complete single-line claimed component identity");
    } else if (property.claimed_manufacturer != entry.component.manufacturer ||
               property.claimed_part_number != entry.component.part_number) {
      result.errors.emplace_back(prefix +
                                 " claimed component identity does not match the catalogue component");
    }
    if (!valid_source(property.source) ||
        !valid_record_text(property.source.title) ||
        !valid_record_text(property.source.url) ||
        !valid_record_text(property.source.revision)) {
      result.errors.emplace_back(prefix +
                                 " requires complete single-line source metadata");
    }
    if (!valid_sha256(property.source_artifact_sha256)) {
      result.errors.emplace_back(prefix +
                                 " requires a lowercase 64-hex SHA-256 source artifact digest");
    }
  }

  if (!result.errors.empty()) return result;

  std::vector<const CatalogQuantityProperty*> ordered_properties;
  ordered_properties.reserve(entry.quantity_properties.size());
  for (const auto& property : entry.quantity_properties) {
    ordered_properties.push_back(&property);
  }
  std::sort(ordered_properties.begin(), ordered_properties.end(),
            [](const auto* lhs, const auto* rhs) {
              return lhs->property_id < rhs->property_id;
            });

  std::ostringstream output;
  output.imbue(std::locale::classic());
  output << "formfactor-catalog-quantity-properties-v3\n";
  append_text_field(output, "catalog-id", entry.catalog_id);
  output << "property-count=" << ordered_properties.size() << '\n';
  for (const auto* property : ordered_properties) {
    output << "property\n";
    append_text_field(output, "id", property->property_id);
    append_text_field(output, "display-name", property->display_name);
    output << "qualifier=" << qualifier_name(property->qualifier) << '\n';
    output << "quantity=" << canonical_quantity_record(property->value) << '\n';
    output << "conditions-present="
           << (property->conditions.has_value() ? 1 : 0) << '\n';
    if (property->conditions.has_value()) {
      append_text_field(output, "conditions", *property->conditions);
    }
    append_text_field(output, "claimed-manufacturer",
                      property->claimed_manufacturer);
    append_text_field(output, "claimed-part-number",
                      property->claimed_part_number);
    append_text_field(output, "source-title", property->source.title);
    append_text_field(output, "source-url", property->source.url);
    append_text_field(output, "source-revision", property->source.revision);
    append_text_field(output, "source-artifact-sha256",
                      property->source_artifact_sha256);
    output << "end-property\n";
  }
  result.canonical_quantity_property_record = output.str();

  return result;
}

}  // namespace formfactor
