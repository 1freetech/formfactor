#include "formfactor/circuit.hpp"

#include <set>

namespace formfactor {

bool CircuitValidationResult::valid() const { return errors.empty(); }

CircuitValidationResult validate_circuit(const std::vector<Net>& nets) {
  CircuitValidationResult result;
  std::set<std::string> net_names;
  std::set<std::string> connected_pins;

  for (const auto& net : nets) {
    if (net.name.empty()) {
      result.errors.emplace_back("net name is required");
    } else if (!net_names.insert(net.name).second) {
      result.errors.emplace_back("duplicate net name: " + net.name);
    }

    if (net.pins.empty()) {
      result.warnings.emplace_back("net has no pins: " + net.name);
    } else if (net.pins.size() == 1) {
      result.warnings.emplace_back("net has only one pin: " + net.name);
    }

    unsigned power_outputs = 0;
    unsigned digital_outputs = 0;
    for (const auto& pin : net.pins) {
      if (pin.component.empty() || pin.pin.empty()) {
        result.errors.emplace_back("pin reference requires component and pin names on net: " + net.name);
        continue;
      }
      const std::string key = pin.component + ":" + pin.pin;
      if (!connected_pins.insert(key).second) {
        result.errors.emplace_back("pin connected to multiple nets: " + key);
      }
      if (pin.type == PinType::PowerOutput) ++power_outputs;
      if (pin.type == PinType::DigitalOutput) ++digital_outputs;
    }

    if (power_outputs > 1) {
      result.errors.emplace_back("multiple power outputs drive net: " + net.name);
    }
    if (digital_outputs > 1) {
      result.errors.emplace_back("multiple digital outputs drive net: " + net.name);
    }
  }
  return result;
}

}  // namespace formfactor
