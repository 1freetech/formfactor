#include "pcbtech/layout.hpp"

#include <cassert>

int main() {
  using namespace pcbtech;
  const LayoutRules rules{100000, 200000, 75000, "fabricator-capability-document"};
  const std::vector<std::string> layers{"F_Cu", "B_Cu"};
  const std::vector<PadGeometry> pads{{"P1", "F_Cu", {0, 0}, 600000}};
  const std::vector<ViaGeometry> vias{{"V1", "F_Cu", "B_Cu", {5000000, 5000000}, 350000, 200000}};
  const std::vector<TraceGeometry> traces{{"T1", "F_Cu", {0, 0}, {5000000, 5000000}, 100000}};
  const auto valid = validate_layout(10000000, 10000000, layers, rules, pads, vias, traces);
  assert(valid.valid() && valid.fabrication_export_allowed());
  assert(valid.canonical_record == validate_layout(10000000, 10000000, layers, rules, pads, vias, traces).canonical_record);

  auto bad_rules = rules; bad_rules.authoritative_source.clear();
  assert(!validate_layout(10000000, 10000000, layers, bad_rules, pads, vias, traces).fabrication_export_allowed());
  auto bad_trace = traces; bad_trace[0].width_nm = 99999;
  assert(!validate_layout(10000000, 10000000, layers, rules, pads, vias, bad_trace).valid());
  bad_trace = traces; bad_trace[0].end = {10000001, 0};
  assert(!validate_layout(10000000, 10000000, layers, rules, pads, vias, bad_trace).valid());
  auto bad_via = vias; bad_via[0].diameter_nm = 349999;
  assert(!validate_layout(10000000, 10000000, layers, rules, pads, bad_via, traces).valid());
  bad_via = vias; bad_via[0].end_layer = "F_Cu";
  assert(!validate_layout(10000000, 10000000, layers, rules, pads, bad_via, traces).valid());
  auto duplicate = traces; duplicate[0].id = "P1";
  assert(!validate_layout(10000000, 10000000, layers, rules, pads, vias, duplicate).valid());
  assert(!validate_layout(0, 10000000, layers, rules, pads, vias, traces).valid());
}
