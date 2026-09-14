#include "formfactor/decoupling.hpp"

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
  return !value.empty() && value.find('\n') == std::string::npos &&
         value.find('\r') == std::string::npos;
}
bool unique_tokens(const std::vector<std::string>& values) {
  std::set<std::string> seen;
  return std::all_of(values.begin(), values.end(), [&](const std::string& value) {
    return token(value) && seen.insert(value).second;
  });
}
}  // namespace

bool DecouplingValidation::valid() const { return errors.empty(); }
bool DecouplingValidation::solver_input_allowed() const {
  return valid() && !canonical_record.empty();
}

DecouplingValidation validate_decoupling_requirements(
    const std::vector<std::string>& net_names,
    const std::vector<std::string>& component_references,
    const std::vector<DecouplingRequirement>& requirements) {
  DecouplingValidation result;
  if (!unique_tokens(net_names)) result.errors.emplace_back("net identifiers must be unique safe tokens");
  if (!unique_tokens(component_references)) result.errors.emplace_back("component references must be unique safe tokens");
  if (requirements.empty()) result.errors.emplace_back("at least one decoupling requirement required");
  const std::set<std::string> nets(net_names.begin(), net_names.end());
  const std::set<std::string> components(component_references.begin(), component_references.end());
  std::set<std::string> ids;
  std::set<std::string> assigned_components;
  std::vector<std::string> records;
  for (const auto& requirement : requirements) {
    if (!token(requirement.id) || !ids.insert(requirement.id).second)
      result.errors.emplace_back("decoupling identifiers must be unique safe tokens");
    if (!nets.contains(requirement.power_net))
      result.errors.emplace_back("unknown decoupling power net: " + requirement.power_net);
    if (!components.contains(requirement.capacitor_reference))
      result.errors.emplace_back("unknown decoupling component: " + requirement.capacitor_reference);
    else if (!assigned_components.insert(requirement.capacitor_reference).second)
      result.errors.emplace_back("component has multiple decoupling requirements: " + requirement.capacitor_reference);
    if (requirement.minimum_capacitance_femtofarads <= 0 ||
        requirement.minimum_voltage_microvolts <= 0 || requirement.maximum_esr_microohms <= 0 ||
        requirement.maximum_esl_femtohenries <= 0 || requirement.maximum_connection_distance_nm < 0)
      result.errors.emplace_back("invalid decoupling electrical or geometry limit: " + requirement.id);
    if (!source(requirement.authoritative_source))
      result.errors.emplace_back("authoritative decoupling source required: " + requirement.id);
    records.push_back("decoupling " + requirement.id + " net=" + requirement.power_net +
        " component=" + requirement.capacitor_reference +
        " min_capacitance_fF=" + std::to_string(requirement.minimum_capacitance_femtofarads) +
        " min_voltage_uV=" + std::to_string(requirement.minimum_voltage_microvolts) +
        " max_esr_uOhm=" + std::to_string(requirement.maximum_esr_microohms) +
        " max_esl_fH=" + std::to_string(requirement.maximum_esl_femtohenries) +
        " max_distance_nm=" + std::to_string(requirement.maximum_connection_distance_nm) +
        " source=" + requirement.authoritative_source);
  }
  if (!result.errors.empty()) return result;
  std::sort(records.begin(), records.end());
  std::ostringstream output;
  output << "formfactor-decoupling-requirements-v1\n";
  for (const auto& record : records) output << record << '\n';
  result.canonical_record = output.str();
  return result;
}

}  // namespace formfactor
