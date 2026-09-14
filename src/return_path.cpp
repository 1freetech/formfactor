#include "formfactor/return_path.hpp"

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

bool ReturnPathValidation::valid() const { return errors.empty(); }
bool ReturnPathValidation::solver_input_allowed() const {
  return valid() && !canonical_record.empty();
}

ReturnPathValidation validate_return_path_constraints(
    const std::vector<std::string>& trace_ids,
    const std::vector<std::string>& net_names,
    const std::vector<ReturnPathConstraint>& constraints) {
  ReturnPathValidation result;
  auto collect = [&](const std::vector<std::string>& values, const std::string& label) {
    std::set<std::string> collected;
    for (const auto& value : values) {
      if (!token(value) || !collected.insert(value).second) {
        result.errors.emplace_back(label + " identifiers must be unique safe tokens");
      }
    }
    return collected;
  };
  const auto traces = collect(trace_ids, "trace");
  const auto nets = collect(net_names, "net");
  if (constraints.empty()) result.errors.emplace_back("at least one return-path constraint required");

  std::set<std::string> ids;
  std::set<std::string> assigned_traces;
  std::vector<std::string> records;
  for (const auto& constraint : constraints) {
    if (!token(constraint.id) || !ids.insert(constraint.id).second) {
      result.errors.emplace_back("return-path constraint identifiers must be unique safe tokens");
    }
    if (!traces.contains(constraint.signal_trace_id)) {
      result.errors.emplace_back("unknown return-path signal trace: " + constraint.signal_trace_id);
    } else if (!assigned_traces.insert(constraint.signal_trace_id).second) {
      result.errors.emplace_back("trace has multiple return-path constraints: " + constraint.signal_trace_id);
    }
    if (!nets.contains(constraint.reference_net)) {
      result.errors.emplace_back("unknown return-path reference net: " + constraint.reference_net);
    }
    if (constraint.maximum_discontinuity_nm < 0) {
      result.errors.emplace_back("maximum return-path discontinuity must be nonnegative: " + constraint.id);
    }
    if (!source(constraint.authoritative_source)) {
      result.errors.emplace_back("authoritative return-path source required: " + constraint.id);
    }
    records.push_back("return-path " + constraint.id + " trace=" + constraint.signal_trace_id +
        " reference_net=" + constraint.reference_net + " max_discontinuity_nm=" +
        std::to_string(constraint.maximum_discontinuity_nm) + " source=" +
        constraint.authoritative_source);
  }
  if (!result.errors.empty()) return result;
  std::sort(records.begin(), records.end());
  std::ostringstream output;
  output << "formfactor-return-path-constraints-v1\n";
  for (const auto& record : records) output << record << '\n';
  result.canonical_record = output.str();
  return result;
}

}  // namespace formfactor
