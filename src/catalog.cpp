#include "pcbtech/catalog.hpp"

#include <unordered_set>

namespace pcbtech {
namespace {

bool valid_source(const Source& source) {
  return !source.title.empty() && !source.url.empty() &&
         !source.revision.empty();
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

CatalogValidationResult validate_catalog_entry(const CatalogEntry& entry) {
  CatalogValidationResult result;

  if (entry.catalog_id.empty()) {
    result.errors.emplace_back("catalog id is required");
  }
  if (entry.display_name.empty()) {
    result.errors.emplace_back("display name is required");
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
    return result;
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
  if (component_pins.size() != entry.component.pin_count) {
    result.errors.emplace_back(
        "explicit pin mappings must cover every logical component pin");
  }

  return result;
}

}  // namespace pcbtech
