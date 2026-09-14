#pragma once

#include "formfactor/component.hpp"
#include "formfactor/quantity.hpp"

#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace formfactor {

enum class ComponentFamily {
  Passive,
  Protection,
  SwitchingAmplification,
  PowerConversion,
  LogicComputeMemory,
  SensorsTiming,
  ConnectionsControls,
  IndicatorsActuators,
  Other
};

struct AssetLink {
  std::string identifier;
  std::string revision;
  Source source;
};

struct PinMapping {
  std::string component_pin;
  std::string footprint_pad;
  std::string simulation_terminal;
};

// Describes what a sourced value claims. This is metadata, not a statement
// that formfactor has independently measured or verified the value.
enum class QuantityValueQualifier { Nominal, Minimum, Typical, Maximum };

struct CatalogQuantityProperty {
  // Stable, project-internal record identifier such as "capacitance.nominal".
  // Identifiers are deliberately separate from user-facing labels.
  std::string property_id;
  std::string display_name;
  Quantity value;
  QuantityValueQualifier qualifier{QuantityValueQualifier::Nominal};
  // Source wording for applicable conditions. Absence remains explicit and
  // must not be interpreted as an unconditional value by higher layers.
  std::optional<std::string> conditions;
  // Exact component identity stated for this claim. These fields prevent a
  // sourced value from being attached to a different catalogue part; they do
  // not authenticate the source or prove the value was transcribed correctly.
  std::string claimed_manufacturer;
  std::string claimed_part_number;
  Source source;
  // Lowercase SHA-256 of the exact source artifact used for transcription.
  // This binds the claim to bytes; it does not authenticate the publisher or
  // prove that the claim was transcribed correctly.
  std::string source_artifact_sha256;
};

struct CatalogQuantityPropertySchema {
  std::string property_id;
  Dimension dimension{Dimension::Dimensionless};
  QuantityValueQualifier qualifier{QuantityValueQualifier::Nominal};
  // Families for which this semantic property is implemented. Absence means
  // unsupported, not physically impossible.
  std::vector<ComponentFamily> applicable_families;

  CatalogQuantityPropertySchema(std::string id, Dimension value_dimension,
                                QuantityValueQualifier value_qualifier)
      : property_id(std::move(id)),
        dimension(value_dimension),
        qualifier(value_qualifier) {}
};

// Returns the implemented semantic contract for a property identifier. An
// absent result means the identifier is unsupported and must remain unknown.
[[nodiscard]] std::optional<CatalogQuantityPropertySchema>
catalog_quantity_property_schema(const std::string& property_id);

struct CatalogEntry {
  std::string catalog_id;
  std::string display_name;
  ComponentFamily primary_family{ComponentFamily::Other};
  std::vector<std::string> function_tags;
  std::vector<std::string> synonyms;
  Component component;
  AssetLink schematic_symbol;
  AssetLink footprint;
  AssetLink physical_model;
  AssetLink simulation_model;
  std::vector<PinMapping> pin_mappings;
  std::vector<CatalogQuantityProperty> quantity_properties;
};

struct CatalogValidationResult {
  std::vector<std::string> errors;
  // Project-internal, versioned replay record. It is emitted only when the
  // complete catalogue entry passes validation.
  std::string canonical_quantity_property_record;
  [[nodiscard]] bool linked_model_ready() const { return errors.empty(); }
  [[nodiscard]] bool quantity_properties_export_allowed() const {
    return linked_model_ready() &&
           !canonical_quantity_property_record.empty();
  }
};

[[nodiscard]] CatalogValidationResult validate_catalog_entry(
    const CatalogEntry& entry);

}  // namespace formfactor
