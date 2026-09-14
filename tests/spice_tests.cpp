#include "formfactor/spice.hpp"

#include <cassert>
#include <cmath>
#include <limits>

int main() {
  using formfactor::SpiceDcVoltageSource;
  using formfactor::SpiceElement;
  using formfactor::SpiceResistor;

  const std::vector<SpiceElement> circuit{
      SpiceResistor{"Rload", "out", "0", 1000.0},
      SpiceDcVoltageSource{"Vinput", "out", "0", 5.0}};
  const auto first = formfactor::export_spice_operating_point("formfactor divider fixture", circuit);
  const auto second = formfactor::export_spice_operating_point("formfactor divider fixture", circuit);
  assert(first.valid());
  assert(first.deck == second.deck);
  assert(first.deck ==
      "formfactor divider fixture\n"
      "Rload out 0 1.00000000000000000e+03\n"
      "Vinput out 0 5.00000000000000000e+00\n"
      ".op\n.end\n");

  assert(!formfactor::export_spice_operating_point("x", {}).valid());
  assert(!formfactor::export_spice_operating_point(
      "x", {SpiceResistor{"R1", "a", "b", 1.0}}).valid());
  assert(!formfactor::export_spice_operating_point(
      "x", {SpiceResistor{"R1", "a", "0", 0.0}}).valid());
  assert(!formfactor::export_spice_operating_point(
      "x", {SpiceResistor{"R1", "a", "0", std::numeric_limits<double>::infinity()}}).valid());
  assert(!formfactor::export_spice_operating_point(
      "x", {SpiceDcVoltageSource{"V1", "a\n.end", "0", 5.0}}).valid());
  assert(!formfactor::export_spice_operating_point(
      "x", {SpiceResistor{"R1", "a", "0", 1.0},
             SpiceResistor{"R1", "b", "0", 2.0}}).valid());
  assert(!formfactor::export_spice_operating_point(
      "bad\n.end", {SpiceResistor{"R1", "a", "0", 1.0}}).valid());
}
