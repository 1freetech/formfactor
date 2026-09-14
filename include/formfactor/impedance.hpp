#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace formfactor {

enum class ImpedanceKind { single_ended, differential };

struct ImpedanceConstraint {
  std::string id;
  ImpedanceKind kind{};
  std::string positive_trace_id;
  std::string negative_trace_id;
  std::int64_t target_milliohms{};
  std::int64_t tolerance_milliohms{};
  std::string authoritative_source;
};

struct ImpedanceValidation {
  std::vector<std::string> errors;
  std::string canonical_record;
  [[nodiscard]] bool valid() const;
  [[nodiscard]] bool engineering_signoff_allowed() const;
};

// Records requirements only. It does not calculate impedance or claim that a
// layout meets a constraint; that requires a separately validated solver.
[[nodiscard]] ImpedanceValidation validate_impedance_constraints(
    const std::vector<std::string>& trace_ids,
    const std::vector<ImpedanceConstraint>& constraints);

}  // namespace formfactor
