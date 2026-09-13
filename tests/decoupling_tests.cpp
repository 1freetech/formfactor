#include "pcbtech/decoupling.hpp"

#include <cassert>

int main() {
  using namespace pcbtech;
  const std::vector<std::string> nets{"VCORE", "GND"};
  const std::vector<std::string> components{"C1", "C2"};
  const std::vector<DecouplingRequirement> valid{
      {"core-bulk", "VCORE", "C2", 100000000000, 2500000, 10000, 1000000,
       2000000, "processor-power-guide"},
      {"core-local", "VCORE", "C1", 100000000, 2500000, 50000, 500000,
       0, "processor-power-guide"}};
  const auto first = validate_decoupling_requirements(nets, components, valid);
  assert(first.valid() && first.solver_input_allowed());
  assert(first.canonical_record ==
         validate_decoupling_requirements(nets, components, valid).canonical_record);
  assert(first.canonical_record.find("max_distance_nm=0") != std::string::npos);

  auto bad = valid; bad[0].minimum_capacitance_femtofarads = 0;
  assert(!validate_decoupling_requirements(nets, components, bad).valid());
  bad = valid; bad[0].minimum_voltage_microvolts = 0;
  assert(!validate_decoupling_requirements(nets, components, bad).valid());
  bad = valid; bad[0].maximum_esr_microohms = 0;
  assert(!validate_decoupling_requirements(nets, components, bad).valid());
  bad = valid; bad[0].maximum_esl_femtohenries = 0;
  assert(!validate_decoupling_requirements(nets, components, bad).valid());
  bad = valid; bad[0].maximum_connection_distance_nm = -1;
  assert(!validate_decoupling_requirements(nets, components, bad).valid());
  bad = valid; bad[0].power_net = "MISSING";
  assert(!validate_decoupling_requirements(nets, components, bad).solver_input_allowed());
  bad = valid; bad[0].capacitor_reference = "C9";
  assert(!validate_decoupling_requirements(nets, components, bad).valid());
  bad = valid; bad[1].capacitor_reference = "C2";
  assert(!validate_decoupling_requirements(nets, components, bad).valid());
  bad = valid; bad[0].authoritative_source.clear();
  assert(!validate_decoupling_requirements(nets, components, bad).solver_input_allowed());
  assert(!validate_decoupling_requirements(nets, components, {}).valid());
}
