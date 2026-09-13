#pragma once

#include "pcbtech/component.hpp"

#include <string>
#include <vector>

namespace pcbtech {

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
};

struct CatalogValidationResult {
  std::vector<std::string> errors;
  [[nodiscard]] bool linked_model_ready() const { return errors.empty(); }
};

[[nodiscard]] CatalogValidationResult validate_catalog_entry(
    const CatalogEntry& entry);

}  // namespace pcbtech
