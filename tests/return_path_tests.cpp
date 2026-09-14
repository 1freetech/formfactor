#include "formfactor/return_path.hpp"

#include <cassert>

int main() {
  using namespace formfactor;
  const std::vector<std::string> traces{"CLK", "DATA"};
  const std::vector<std::string> nets{"GND", "VCC"};
  const std::vector<ReturnPathConstraint> valid{
      {"clk-return", "CLK", "GND", 0, "interface-design-guide"},
      {"data-return", "DATA", "GND", 250000, "interface-design-guide"}};
  const auto first = validate_return_path_constraints(traces, nets, valid);
  assert(first.valid() && first.solver_input_allowed());
  assert(first.canonical_record == validate_return_path_constraints(traces, nets, valid).canonical_record);

  auto bad = valid; bad[0].maximum_discontinuity_nm = -1;
  assert(!validate_return_path_constraints(traces, nets, bad).valid());
  bad = valid; bad[0].signal_trace_id = "MISSING";
  assert(!validate_return_path_constraints(traces, nets, bad).solver_input_allowed());
  bad = valid; bad[0].reference_net = "MISSING";
  assert(!validate_return_path_constraints(traces, nets, bad).valid());
  bad = valid; bad[0].authoritative_source.clear();
  assert(!validate_return_path_constraints(traces, nets, bad).solver_input_allowed());
  bad = valid; bad[1].signal_trace_id = "CLK";
  assert(!validate_return_path_constraints(traces, nets, bad).valid());
  assert(!validate_return_path_constraints(traces, nets, {}).valid());
}
