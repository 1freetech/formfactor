#pragma once

#include <optional>
#include <string>
#include <vector>

namespace pcbtech {

enum class StackupLayerKind { copper, dielectric };

struct StackupLayer {
  std::string name;
  StackupLayerKind kind{};
  double thickness_micrometres{};
  std::string material;
  std::string authoritative_source;
  std::optional<double> relative_permittivity;
  std::optional<double> loss_tangent;
};

struct StackupValidation {
  std::vector<std::string> errors;
  std::string canonical_record;
  [[nodiscard]] bool valid() const;
  [[nodiscard]] bool fabrication_export_allowed() const;
};

// Validates caller-supplied physical structure without selecting materials or
// inventing fabrication limits. Layer order is top-to-bottom.
[[nodiscard]] StackupValidation validate_stackup(
    const std::string& name, const std::vector<StackupLayer>& layers);

}  // namespace pcbtech
