#include "pcbtech/layout.hpp"

#include <algorithm>
#include <set>
#include <sstream>
#include <tuple>

namespace pcbtech {
namespace {
bool single_line(const std::string& value) {
  return !value.empty() && value.find('\n') == std::string::npos && value.find('\r') == std::string::npos;
}
bool token(const std::string& value) {
  return !value.empty() && std::all_of(value.begin(), value.end(), [](const unsigned char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.';
  });
}
bool inside(const PointNm p, const std::int64_t width, const std::int64_t height) {
  return p.x >= 0 && p.y >= 0 && p.x <= width && p.y <= height;
}
}  // namespace

bool LayoutValidation::valid() const { return errors.empty(); }
bool LayoutValidation::fabrication_export_allowed() const { return valid() && !canonical_record.empty(); }

LayoutValidation validate_layout(
    const std::int64_t board_width_nm, const std::int64_t board_height_nm,
    const std::vector<std::string>& copper_layers, const LayoutRules& rules,
    const std::vector<PadGeometry>& pads, const std::vector<ViaGeometry>& vias,
    const std::vector<TraceGeometry>& traces) {
  LayoutValidation result;
  if (board_width_nm <= 0 || board_height_nm <= 0) result.errors.emplace_back("board dimensions must be positive nanometres");
  if (rules.minimum_trace_width_nm <= 0 || rules.minimum_via_drill_nm <= 0 ||
      rules.minimum_annular_ring_nm < 0) result.errors.emplace_back("layout rules contain invalid nanometre values");
  if (!single_line(rules.authoritative_source)) result.errors.emplace_back("authoritative layout-rule source required");

  std::set<std::string> layers;
  for (const auto& layer : copper_layers) {
    if (!token(layer) || !layers.insert(layer).second) result.errors.emplace_back("copper layer names must be unique safe tokens");
  }
  if (layers.empty()) result.errors.emplace_back("at least one copper layer required");
  std::set<std::string> ids;
  auto identity = [&](const std::string& id) {
    if (!token(id) || !ids.insert(id).second) result.errors.emplace_back("geometry identifiers must be unique safe tokens");
  };
  auto known_layer = [&](const std::string& layer, const std::string& id) {
    if (!layers.contains(layer)) result.errors.emplace_back("unknown copper layer on: " + id);
  };

  for (const auto& pad : pads) {
    identity(pad.id); known_layer(pad.layer, pad.id);
    if (!inside(pad.centre, board_width_nm, board_height_nm)) result.errors.emplace_back("pad outside board: " + pad.id);
    if (pad.diameter_nm <= 0) result.errors.emplace_back("pad diameter must be positive: " + pad.id);
  }
  for (const auto& via : vias) {
    identity(via.id); known_layer(via.start_layer, via.id); known_layer(via.end_layer, via.id);
    if (via.start_layer == via.end_layer) result.errors.emplace_back("via must span distinct copper layers: " + via.id);
    if (!inside(via.centre, board_width_nm, board_height_nm)) result.errors.emplace_back("via outside board: " + via.id);
    if (via.drill_nm < rules.minimum_via_drill_nm) result.errors.emplace_back("via drill below sourced rule: " + via.id);
    if (via.drill_nm <= 0 || via.diameter_nm <= via.drill_nm ||
        (via.diameter_nm > via.drill_nm &&
         (via.diameter_nm - via.drill_nm) / 2 < rules.minimum_annular_ring_nm)) {
      result.errors.emplace_back("via annular ring below sourced rule: " + via.id);
    }
  }
  for (const auto& trace : traces) {
    identity(trace.id); known_layer(trace.layer, trace.id);
    if (!inside(trace.start, board_width_nm, board_height_nm) ||
        !inside(trace.end, board_width_nm, board_height_nm)) result.errors.emplace_back("trace endpoint outside board: " + trace.id);
    if (trace.start.x == trace.end.x && trace.start.y == trace.end.y) result.errors.emplace_back("trace must have nonzero length: " + trace.id);
    if (trace.width_nm < rules.minimum_trace_width_nm) result.errors.emplace_back("trace width below sourced rule: " + trace.id);
  }
  if (!result.errors.empty()) return result;

  std::vector<std::string> records;
  for (const auto& pad : pads) records.push_back("pad " + pad.id + " " + pad.layer + " " + std::to_string(pad.centre.x) + " " + std::to_string(pad.centre.y) + " " + std::to_string(pad.diameter_nm));
  for (const auto& via : vias) records.push_back("via " + via.id + " " + via.start_layer + " " + via.end_layer + " " + std::to_string(via.centre.x) + " " + std::to_string(via.centre.y) + " " + std::to_string(via.diameter_nm) + " " + std::to_string(via.drill_nm));
  for (const auto& trace : traces) records.push_back("trace " + trace.id + " " + trace.layer + " " + std::to_string(trace.start.x) + " " + std::to_string(trace.start.y) + " " + std::to_string(trace.end.x) + " " + std::to_string(trace.end.y) + " " + std::to_string(trace.width_nm));
  std::sort(records.begin(), records.end());
  std::ostringstream output;
  output << "pcbtech-layout-v1 " << board_width_nm << ' ' << board_height_nm << " nm source=" << rules.authoritative_source << '\n';
  for (const auto& record : records) output << record << '\n';
  result.canonical_record = output.str();
  return result;
}

}  // namespace pcbtech
