#include "formfactor/impedance.hpp"

#include <algorithm>
#include <set>
#include <sstream>

namespace formfactor {
namespace {
bool token(const std::string& value) {
  return !value.empty() && std::all_of(value.begin(), value.end(), [](const unsigned char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.';
  });
}
bool source(const std::string& value) {
  return !value.empty() && value.find('\n') == std::string::npos && value.find('\r') == std::string::npos;
}
}  // namespace

bool ImpedanceValidation::valid() const { return errors.empty(); }
bool ImpedanceValidation::engineering_signoff_allowed() const {
  return valid() && !canonical_record.empty();
}

ImpedanceValidation validate_impedance_constraints(
    const std::vector<std::string>& trace_ids,
    const std::vector<ImpedanceConstraint>& constraints) {
  ImpedanceValidation result;
  std::set<std::string> known_traces;
  for (const auto& trace : trace_ids) {
    if (!token(trace) || !known_traces.insert(trace).second) {
      result.errors.emplace_back("trace identifiers must be unique safe tokens");
    }
  }
  if (constraints.empty()) result.errors.emplace_back("at least one impedance constraint required");

  std::set<std::string> ids;
  std::set<std::string> assigned_traces;
  std::vector<std::string> records;
  for (const auto& constraint : constraints) {
    if (!token(constraint.id) || !ids.insert(constraint.id).second) {
      result.errors.emplace_back("impedance constraint identifiers must be unique safe tokens");
    }
    if (constraint.target_milliohms <= 0 || constraint.tolerance_milliohms < 0 ||
        constraint.tolerance_milliohms >= constraint.target_milliohms) {
      result.errors.emplace_back("impedance target/tolerance interval must remain positive: " + constraint.id);
    }
    if (!source(constraint.authoritative_source)) {
      result.errors.emplace_back("authoritative impedance source required: " + constraint.id);
    }
    auto assign = [&](const std::string& trace) {
      if (!known_traces.contains(trace)) result.errors.emplace_back("unknown constrained trace: " + trace);
      else if (!assigned_traces.insert(trace).second) result.errors.emplace_back("trace has multiple impedance constraints: " + trace);
    };
    assign(constraint.positive_trace_id);
    if (constraint.kind == ImpedanceKind::differential) {
      if (constraint.negative_trace_id == constraint.positive_trace_id) {
        result.errors.emplace_back("differential constraint requires distinct traces: " + constraint.id);
      } else {
        assign(constraint.negative_trace_id);
      }
    } else if (!constraint.negative_trace_id.empty()) {
      result.errors.emplace_back("single-ended constraint cannot name a negative trace: " + constraint.id);
    }
    records.push_back(std::string(constraint.kind == ImpedanceKind::differential ? "differential " : "single-ended ") +
        constraint.id + " positive=" + constraint.positive_trace_id + " negative=" +
        (constraint.negative_trace_id.empty() ? "none" : constraint.negative_trace_id) +
        " target_mOhm=" + std::to_string(constraint.target_milliohms) +
        " tolerance_mOhm=" + std::to_string(constraint.tolerance_milliohms) +
        " source=" + constraint.authoritative_source);
  }
  if (!result.errors.empty()) return result;
  std::sort(records.begin(), records.end());
  std::ostringstream output;
  output << "formfactor-impedance-constraints-v1\n";
  for (const auto& record : records) output << record << '\n';
  result.canonical_record = output.str();
  return result;
}

}  // namespace formfactor
