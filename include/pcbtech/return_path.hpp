#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace pcbtech {

struct ReturnPathConstraint {
  std::string id;
  std::string signal_trace_id;
  std::string reference_net;
  std::int64_t maximum_discontinuity_nm{};
  std::string authoritative_source;
};

struct ReturnPathValidation {
  std::vector<std::string> errors;
  std::string canonical_record;
  [[nodiscard]] bool valid() const;
  [[nodiscard]] bool solver_input_allowed() const;
};

// Validates return-path requirements, not physical plane continuity.
[[nodiscard]] ReturnPathValidation validate_return_path_constraints(
    const std::vector<std::string>& trace_ids,
    const std::vector<std::string>& net_names,
    const std::vector<ReturnPathConstraint>& constraints);

}  // namespace pcbtech
