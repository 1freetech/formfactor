#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace pcbtech {

struct PointNm { std::int64_t x{}; std::int64_t y{}; };
struct PadGeometry { std::string id; std::string layer; PointNm centre; std::int64_t diameter_nm{}; std::string net; };
struct ViaGeometry { std::string id; std::string start_layer; std::string end_layer; PointNm centre; std::int64_t diameter_nm{}; std::int64_t drill_nm{}; std::string net; };
struct TraceGeometry {
  std::string id;
  std::string layer;
  PointNm start;
  PointNm end;
  std::int64_t width_nm{};
  std::string net;
  std::optional<std::int64_t> current_load_microamps;
  std::optional<std::int64_t> current_limit_microamps;
  std::string current_limit_source;
};

struct LayoutRules {
  std::int64_t minimum_trace_width_nm{};
  std::int64_t minimum_via_drill_nm{};
  std::int64_t minimum_annular_ring_nm{};
  std::string authoritative_source;
  std::int64_t minimum_clearance_nm{};
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
