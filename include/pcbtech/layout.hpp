#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace pcbtech {

struct PointNm { std::int64_t x{}; std::int64_t y{}; };
struct PadGeometry { std::string id; std::string layer; PointNm centre; std::int64_t diameter_nm{}; };
struct ViaGeometry { std::string id; std::string start_layer; std::string end_layer; PointNm centre; std::int64_t diameter_nm{}; std::int64_t drill_nm{}; };
struct TraceGeometry { std::string id; std::string layer; PointNm start; PointNm end; std::int64_t width_nm{}; };

struct LayoutRules {
  std::int64_t minimum_trace_width_nm{};
  std::int64_t minimum_via_drill_nm{};
  std::int64_t minimum_annular_ring_nm{};
  std::string authoritative_source;
};

struct LayoutValidation {
  std::vector<std::string> errors;
  std::string canonical_record;
  [[nodiscard]] bool valid() const;
  [[nodiscard]] bool fabrication_export_allowed() const;
};

[[nodiscard]] LayoutValidation validate_layout(
    std::int64_t board_width_nm, std::int64_t board_height_nm,
    const std::vector<std::string>& copper_layers, const LayoutRules& rules,
    const std::vector<PadGeometry>& pads, const std::vector<ViaGeometry>& vias,
    const std::vector<TraceGeometry>& traces);

}  // namespace pcbtech
