#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace formfactor {

struct DecouplingRequirement {
  std::string id;
  std::string power_net;
  std::string capacitor_reference;
  std::int64_t minimum_capacitance_femtofarads{};
  std::int64_t minimum_voltage_microvolts{};
  std::int64_t maximum_esr_microohms{};
  std::int64_t maximum_esl_femtohenries{};
  std::int64_t maximum_connection_distance_nm{};
  std::string authoritative_source;
};

struct DecouplingValidation {
  std::vector<std::string> errors;
  std::string canonical_record;
  [[nodiscard]] bool valid() const;
  [[nodiscard]] bool solver_input_allowed() const;
};

// Validates sourced requirements only. It does not calculate impedance or
// claim that a placed capacitor satisfies the requirement.
[[nodiscard]] DecouplingValidation validate_decoupling_requirements(
    const std::vector<std::string>& net_names,
    const std::vector<std::string>& component_references,
    const std::vector<DecouplingRequirement>& requirements);

}  // namespace formfactor
