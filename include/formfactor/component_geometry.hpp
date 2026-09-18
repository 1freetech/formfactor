#pragma once

#include <string_view>
#include <vector>

namespace formfactor {

// Presentation-only geometry recipes for the interactive 3D client.
// Units are normalized display units, not mechanical dimensions.
// Exact package dimensions must come from a verified package/datasheet source.

enum class VisualPrimitiveKind {
  Box,
  Cylinder,
  Sphere,
  Disc,
  Lead,
};

struct VisualPrimitive {
  VisualPrimitiveKind kind{};
  double size_x{};
  double size_y{};
  double size_z{};
  double offset_x{};
  double offset_y{};
  double offset_z{};
  double rotation_x_degrees{};
  double rotation_y_degrees{};
  double rotation_z_degrees{};
};

struct ComponentVisualRecipe {
  std::string_view component_id;
  std::string_view package_style;
  bool visual_only{true};
  std::vector<VisualPrimitive> primitives;
};

const std::vector<ComponentVisualRecipe>& component_visual_recipes();
const ComponentVisualRecipe* find_component_visual_recipe(
    std::string_view component_id);

}  // namespace formfactor
