#include "pcbtech/pdn.hpp"

#include <algorithm>
#include <numeric>
#include <set>
#include <sstream>

namespace pcbtech {
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

bool PdnValidation::valid() const { return errors.empty(); }
bool PdnValidation::solver_input_allowed() const { return valid() && !canonical_record.empty(); }

PdnValidation validate_pdn_constraints(const std::vector<std::string>& net_names,
                                       const std::vector<PdnConstraint>& constraints) {
  PdnValidation result;
  std::set<std::string> nets;
  for (const auto& net : net_names) {
    if (!token(net) || !nets.insert(net).second) result.errors.emplace_back("net identifiers must be unique safe tokens");
  }
  if (constraints.empty()) result.errors.emplace_back("at least one PDN constraint required");
  std::set<std::string> ids;
  std::set<std::string> assigned_nets;
  std::vector<std::string> records;
  for (const auto& constraint : constraints) {
    if (!token(constraint.id) || !ids.insert(constraint.id).second) result.errors.emplace_back("PDN constraint identifiers must be unique safe tokens");
    if (!nets.contains(constraint.power_net)) result.errors.emplace_back("unknown PDN power net: " + constraint.power_net);
    else if (!assigned_nets.insert(constraint.power_net).second) result.errors.emplace_back("power net has multiple PDN constraints: " + constraint.power_net);
    if (constraint.maximum_ripple_microvolts <= 0 || constraint.load_step_microamps <= 0 ||
        constraint.analysis_bandwidth_hz <= 0) result.errors.emplace_back("PDN electrical inputs must be positive: " + constraint.id);
    if (!source(constraint.authoritative_source)) result.errors.emplace_back("authoritative PDN source required: " + constraint.id);
    if (constraint.maximum_ripple_microvolts > 0 && constraint.load_step_microamps > 0) {
      const auto divisor = std::gcd(constraint.maximum_ripple_microvolts, constraint.load_step_microamps);
      records.push_back("pdn " + constraint.id + " net=" + constraint.power_net +
          " ripple_uV=" + std::to_string(constraint.maximum_ripple_microvolts) +
          " load_step_uA=" + std::to_string(constraint.load_step_microamps) +
          " target_ohms=" + std::to_string(constraint.maximum_ripple_microvolts / divisor) + "/" +
          std::to_string(constraint.load_step_microamps / divisor) +
          " bandwidth_Hz=" + std::to_string(constraint.analysis_bandwidth_hz) +
          " source=" + constraint.authoritative_source);
    }
  }
  if (!result.errors.empty()) return result;
  std::sort(records.begin(), records.end());
  std::ostringstream output;
  output << "pcbtech-pdn-constraints-v1 equation=Z_target=deltaV/deltaI\n";
  for (const auto& record : records) output << record << '\n';
  result.canonical_record = output.str();
  return result;
}

}  // namespace pcbtech
