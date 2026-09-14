#include "formfactor/circuit.hpp"

#include <cassert>
#include <string>

namespace {

bool contains(const std::vector<std::string>& messages, const std::string& text) {
  for (const auto& message : messages) {
    if (message.find(text) != std::string::npos) return true;
  }
  return false;
}

}  // namespace

int main() {
  using formfactor::Net;
  using formfactor::PinType;

  const std::vector<Net> valid{
      {"VCC", {{"REG1", "OUT", PinType::PowerOutput}, {"U1", "VCC", PinType::PowerInput}}},
      {"DATA", {{"U1", "OUT", PinType::DigitalOutput}, {"U2", "IN", PinType::DigitalInput}}}};
  assert(formfactor::validate_circuit(valid).valid());

  const std::vector<Net> power_conflict{{
      "VCC", {{"REG1", "OUT", PinType::PowerOutput}, {"REG2", "OUT", PinType::PowerOutput}}}};
  const auto power_result = formfactor::validate_circuit(power_conflict);
  assert(!power_result.valid());
  assert(contains(power_result.errors, "multiple power outputs"));

  const std::vector<Net> output_conflict{{
      "DATA", {{"U1", "OUT", PinType::DigitalOutput}, {"U2", "OUT", PinType::DigitalOutput}}}};
  assert(!formfactor::validate_circuit(output_conflict).valid());

  const std::vector<Net> reused_pin{
      {"A", {{"U1", "1", PinType::Passive}, {"R1", "1", PinType::Passive}}},
      {"B", {{"U1", "1", PinType::Passive}, {"R2", "1", PinType::Passive}}}};
  const auto reused_result = formfactor::validate_circuit(reused_pin);
  assert(!reused_result.valid());
  assert(contains(reused_result.errors, "multiple nets"));

  const auto dangling = formfactor::validate_circuit({{"SENSE", {{"U1", "ADC", PinType::DigitalInput}}}});
  assert(dangling.valid());
  assert(contains(dangling.warnings, "only one pin"));

  const auto unnamed = formfactor::validate_circuit({{"", {{"U1", "1", PinType::Passive}}}});
  assert(!unnamed.valid());
}
