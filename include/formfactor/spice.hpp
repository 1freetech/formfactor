#pragma once

#include <string>
#include <variant>
#include <vector>

namespace formfactor {

struct SpiceResistor {
  std::string reference;
  std::string positive_node;
  std::string negative_node;
  double resistance_ohms{};
};

struct SpiceDcVoltageSource {
  std::string reference;
  std::string positive_node;
  std::string negative_node;
  double voltage_volts{};
};

using SpiceElement = std::variant<SpiceResistor, SpiceDcVoltageSource>;

struct SpiceDeckResult {
  std::string deck;
  std::vector<std::string> errors;
  [[nodiscard]] bool valid() const;
};

// Produces a canonical ngspice-compatible operating-point deck. This adapter
// serializes caller-supplied values only; it does not infer component models.
[[nodiscard]] SpiceDeckResult export_spice_operating_point(
    const std::string& title, const std::vector<SpiceElement>& elements);

}  // namespace formfactor
