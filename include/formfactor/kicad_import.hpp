#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace formfactor {

struct KicadLegacyComponent {
  std::string reference;
  std::string value;
  std::string symbol_library_id;
  std::string footprint_library_id;
};

struct KicadLegacyImport {
  std::vector<KicadLegacyComponent> components;
  std::vector<std::string> errors;
  [[nodiscard]] bool valid() const;
};

// Imports component identity links from KiCad's legacy Eeschema .sch format.
// This importer preserves explicit source fields only. It does not infer pin
// mappings, ratings, simulation models, or electrical correctness.
[[nodiscard]] KicadLegacyImport parse_legacy_kicad_schematic(std::string_view source);

}  // namespace formfactor
