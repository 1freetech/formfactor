#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace pcbtech {

struct PdnConstraint {
  std::string id;
  std::string power_net;
  std::int64_t maximum_ripple_microvolts{};
  std::int64_t load_step_microamps{};
  std::int64_t analysis_bandwidth_hz{};
  std::string authoritative_source;
};

struct PdnValidation {
  std::vector<std::string> errors;
  std::string canonical_record;
  [[nodiscard]] bool valid() const;
  [[nodiscard]] bool solver_input_allowed() const;
};

// Encodes Z_target = delta-V / delta-I as an exact reduced rational number.
// Equal micro-prefixes cancel, leaving ohms. No PDN response is calculated.
[[nodiscard]] PdnValidation validate_pdn_constraints(
    const std::vector<std::string>& net_names,
    const std::vector<PdnConstraint>& constraints);

}  // namespace pcbtech
