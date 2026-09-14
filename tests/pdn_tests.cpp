#include "formfactor/pdn.hpp"

#include <cassert>

int main() {
  using namespace formfactor;
  const std::vector<std::string> nets{"VCORE", "VIO"};
  const std::vector<PdnConstraint> valid{
      {"core-pdn", "VCORE", 50000, 10000000, 100000000, "processor-power-spec"},
      {"io-pdn", "VIO", 33000, 3000000, 50000000, "interface-power-spec"}};
  const auto first = validate_pdn_constraints(nets, valid);
  assert(first.valid() && first.solver_input_allowed());
  assert(first.canonical_record == validate_pdn_constraints(nets, valid).canonical_record);
  assert(first.canonical_record.find("target_ohms=1/200") != std::string::npos);

  auto bad = valid; bad[0].maximum_ripple_microvolts = 0;
  assert(!validate_pdn_constraints(nets, bad).valid());
  bad = valid; bad[0].load_step_microamps = 0;
  assert(!validate_pdn_constraints(nets, bad).valid());
  bad = valid; bad[0].analysis_bandwidth_hz = 0;
  assert(!validate_pdn_constraints(nets, bad).valid());
  bad = valid; bad[0].power_net = "MISSING";
  assert(!validate_pdn_constraints(nets, bad).solver_input_allowed());
  bad = valid; bad[0].authoritative_source.clear();
  assert(!validate_pdn_constraints(nets, bad).solver_input_allowed());
  bad = valid; bad[1].power_net = "VCORE";
  assert(!validate_pdn_constraints(nets, bad).valid());
  assert(!validate_pdn_constraints(nets, {}).valid());
}
