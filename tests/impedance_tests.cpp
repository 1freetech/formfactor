#include "formfactor/impedance.hpp"

#include <cassert>

int main() {
  using namespace formfactor;
  const std::vector<std::string> traces{"CLK", "USB_P", "USB_N"};
  const std::vector<ImpedanceConstraint> valid{
      {"clock-z", ImpedanceKind::single_ended, "CLK", "", 50000, 5000, "interface-specification"},
      {"usb-zdiff", ImpedanceKind::differential, "USB_P", "USB_N", 90000, 9000, "interface-specification"}};
  const auto first = validate_impedance_constraints(traces, valid);
  assert(first.valid() && first.engineering_signoff_allowed());
  assert(first.canonical_record == validate_impedance_constraints(traces, valid).canonical_record);

  auto bad = valid; bad[0].target_milliohms = 0;
  assert(!validate_impedance_constraints(traces, bad).valid());
  bad = valid; bad[0].tolerance_milliohms = 50000;
  assert(!validate_impedance_constraints(traces, bad).valid());
  bad = valid; bad[1].negative_trace_id = "USB_P";
  assert(!validate_impedance_constraints(traces, bad).valid());
  bad = valid; bad[0].positive_trace_id = "MISSING";
  assert(!validate_impedance_constraints(traces, bad).engineering_signoff_allowed());
  bad = valid; bad[0].authoritative_source.clear();
  assert(!validate_impedance_constraints(traces, bad).engineering_signoff_allowed());
  bad = valid; bad[1].positive_trace_id = "CLK";
  assert(!validate_impedance_constraints(traces, bad).valid());
  assert(!validate_impedance_constraints(traces, {}).valid());
}
