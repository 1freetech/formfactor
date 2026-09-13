#include "pcbtech/circuit.hpp"

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
  using pcbtech::Net;
  using pcbtech::PinType;

  const std::vector<Net> valid{
      {"VCC", {{"REG1", "OUT", PinType::PowerOutput}, {"U1", "VCC", PinType::PowerInput}}},
      {"DATA", {{"U1", "OUT", PinType::DigitalOutput}, {"U2", "IN", PinType::DigitalInput}}}};
  assert(pcbtech::validate_circuit(valid).valid());

  const std::vector<Net> power_conflict{{
      "VCC", {{"REG1", "OUT", PinType::PowerOutput}, {"REG2", "OUT", PinType::PowerOutput}}}};
  const auto power_result = pcbtech::validate_circuit(power_conflict);
  assert(!power_result.valid());
  assert(contains(power_result.errors, "multiple power outputs"));

  const std::vector<Net> output_conflict{{
      "DATA", {{"U1", "OUT", PinType::DigitalOutput}, {"U2", "OUT", PinType::DigitalOutput}}}};
  assert(!pcbtech::validate_circuit(output_conflict).valid());

  const std::vector<Net> reused_pin{
      {"A", {{"U1", "1", PinType::Passive}, {"R1", "1", PinType::Passive}}},
      {"B", {{"U1", "1", PinType::Passive}, {"R2", "1", PinType::Passive}}}};
  const auto reused_result = pcbtech::validate_circuit(reused_pin);
  assert(!reused_result.valid());
  assert(contains(reused_result.errors, "multiple nets"));

  const auto dangling = pcbtech::validate_circuit({{"SENSE", {{"U1", "ADC", PinType::DigitalInput}}}});
  assert(dangling.valid());
  assert(contains(dangling.warnings, "only one pin"));

  const auto unnamed = pcbtech::validate_circuit({{"", {{"U1", "1", PinType::Passive}}}});
  assert(!unnamed.valid());
}
