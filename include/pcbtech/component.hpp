#pragma once

#include <string>
#include <vector>

namespace pcbtech {

enum class AccuracyTier { VisualOnly, Partial, Verified };

struct Source {
  std::string title;
  std::string url;
  std::string revision;
};

struct Component {
  std::string manufacturer;
  std::string part_number;
  std::string package;
  unsigned pin_count{};
  double max_voltage_v{};
  double max_current_a{};
  double max_junction_c{};
  bool has_simulation_model{};
  AccuracyTier requested_tier{AccuracyTier::VisualOnly};
  std::vector<Source> sources;
};

struct ValidationResult {
  AccuracyTier effective_tier{AccuracyTier::VisualOnly};
  std::vector<std::string> errors;
  [[nodiscard]] bool export_allowed() const;
};

[[nodiscard]] ValidationResult validate(const Component& component);

}  // namespace pcbtech

