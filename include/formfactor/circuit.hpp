#pragma once

#include <string>
#include <vector>

namespace formfactor {

enum class PinType { Passive, PowerInput, PowerOutput, DigitalInput, DigitalOutput, OpenDrain };

struct PinRef {
  std::string component;
  std::string pin;
  PinType type{PinType::Passive};
};

struct Net {
  std::string name;
  std::vector<PinRef> pins;
};

struct CircuitValidationResult {
  std::vector<std::string> errors;
  std::vector<std::string> warnings;
  [[nodiscard]] bool valid() const;
};

// Performs topology-only electrical-rule checks. It never infers voltage,
// current, logic level, or component behavior that is absent from the input.
[[nodiscard]] CircuitValidationResult validate_circuit(const std::vector<Net>& nets);

}  // namespace formfactor
