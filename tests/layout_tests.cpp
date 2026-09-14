#include "formfactor/layout.hpp"

#include <cassert>

int main() {
  using namespace formfactor;
  const LayoutRules rules{100000, 200000, 75000, "fabricator-capability-document", 100000};
  const std::vector<std::string> layers{"F_Cu", "B_Cu"};
  const std::vector<PadGeometry> pads{{"P1", "F_Cu", {0, 0}, 600000, "GND"}};
  const std::vector<ViaGeometry> vias{{"V1", "F_Cu", "B_Cu", {5000000, 5000000}, 350000, 200000, "GND"}};
  const std::vector<TraceGeometry> traces{{"T1", "F_Cu", {0, 0}, {5000000, 5000000}, 100000, "GND", 500000, 500000, "validated-ampacity-source"}};
  const auto valid = validate_layout(10000000, 10000000, layers, rules, pads, vias, traces);
  assert(valid.valid() && valid.fabrication_export_allowed());
  assert(valid.canonical_record == validate_layout(10000000, 10000000, layers, rules, pads, vias, traces).canonical_record);

  auto bad_rules = rules; bad_rules.authoritative_source.clear();
  assert(!validate_layout(10000000, 10000000, layers, bad_rules, pads, vias, traces).fabrication_export_allowed());
  auto bad_trace = traces; bad_trace[0].width_nm = 99999;
  assert(!validate_layout(10000000, 10000000, layers, rules, pads, vias, bad_trace).valid());
  bad_trace = traces; bad_trace[0].end = {10000001, 0};
  assert(!validate_layout(10000000, 10000000, layers, rules, pads, vias, bad_trace).valid());
  bad_trace = traces; bad_trace[0].current_load_microamps = 500001;
  assert(!validate_layout(10000000, 10000000, layers, rules, pads, vias, bad_trace).valid());
  bad_trace = traces; bad_trace[0].current_load_microamps.reset();
  assert(!validate_layout(10000000, 10000000, layers, rules, pads, vias, bad_trace).fabrication_export_allowed());
  bad_trace = traces; bad_trace[0].current_limit_source.clear();
  assert(!validate_layout(10000000, 10000000, layers, rules, pads, vias, bad_trace).fabrication_export_allowed());
  auto bad_via = vias; bad_via[0].diameter_nm = 349999;
  assert(!validate_layout(10000000, 10000000, layers, rules, pads, bad_via, traces).valid());
  bad_via = vias; bad_via[0].end_layer = "F_Cu";
  assert(!validate_layout(10000000, 10000000, layers, rules, pads, bad_via, traces).valid());
  auto duplicate = traces; duplicate[0].id = "P1";
  assert(!validate_layout(10000000, 10000000, layers, rules, pads, vias, duplicate).valid());
  auto near_pad = pads;
  near_pad.push_back({"P2", "F_Cu", {799999, 0}, 800000, "VCC"});
  assert(!validate_layout(10000000, 10000000, layers, rules, near_pad, vias, traces).valid());
  near_pad[1].centre.x = 800000;  // exact clearance boundary is valid
  assert(validate_layout(10000000, 10000000, layers, rules, near_pad, vias, traces).valid());
  near_pad[1].centre.x = 0; near_pad[1].net = "GND";  // same-net overlap is not a clearance violation
  assert(validate_layout(10000000, 10000000, layers, rules, near_pad, vias, traces).valid());
  const std::vector<TraceGeometry> clearance_trace{{"TC", "F_Cu", {1000000, 2000000},
      {5000000, 2000000}, 100000, "VCC", 1, 1, "validated-ampacity-source"}};
  std::vector<PadGeometry> trace_pad{{"PC", "F_Cu", {3000000, 2449999}, 600000, "GND"}};
  assert(!validate_layout(10000000, 10000000, layers, rules, trace_pad, {}, clearance_trace).valid());
  trace_pad[0].centre.y = 2450000;  // exact capsule clearance boundary is valid
  assert(validate_layout(10000000, 10000000, layers, rules, trace_pad, {}, clearance_trace).valid());
  trace_pad[0].centre = {5450000, 2000000};  // endpoint distance also uses exact clearance
  assert(validate_layout(10000000, 10000000, layers, rules, trace_pad, {}, clearance_trace).valid());
  trace_pad[0].centre.x = 5449999;
  assert(!validate_layout(10000000, 10000000, layers, rules, trace_pad, {}, clearance_trace).valid());
  trace_pad[0].centre = {3000000, 2000000}; trace_pad[0].net = "VCC";
  assert(validate_layout(10000000, 10000000, layers, rules, trace_pad, {}, clearance_trace).valid());
  assert(!validate_layout(1000000001, 10000000, layers, rules, pads, vias, traces).valid());
  assert(!validate_layout(0, 10000000, layers, rules, pads, vias, traces).valid());
}
