#include "formfactor/component_geometry.hpp"

#include <algorithm>

namespace formfactor {
namespace {

VisualPrimitive box(double x, double y, double z, double ox = 0.0,
                    double oy = 0.0, double oz = 0.0) {
  return {VisualPrimitiveKind::Box, x, y, z, ox, oy, oz, 0.0, 0.0, 0.0};
}

VisualPrimitive cylinder(double diameter, double height, double ox = 0.0,
                         double oy = 0.0, double oz = 0.0,
                         double rz = 0.0) {
  return {VisualPrimitiveKind::Cylinder, diameter, height, diameter,
          ox, oy, oz, 0.0, 0.0, rz};
}

VisualPrimitive sphere(double diameter, double ox = 0.0, double oy = 0.0,
                       double oz = 0.0) {
  return {VisualPrimitiveKind::Sphere, diameter, diameter, diameter,
          ox, oy, oz, 0.0, 0.0, 0.0};
}

VisualPrimitive disc(double diameter, double thickness, double ox = 0.0,
                     double oy = 0.0, double oz = 0.0,
                     double rx = 90.0) {
  return {VisualPrimitiveKind::Disc, diameter, thickness, diameter,
          ox, oy, oz, rx, 0.0, 0.0};
}

VisualPrimitive lead(double x, double y, double z, double ox = 0.0,
                     double oy = 0.0, double oz = 0.0) {
  return {VisualPrimitiveKind::Lead, x, y, z, ox, oy, oz, 0.0, 0.0, 0.0};
}

std::vector<ComponentVisualRecipe> make_catalog() {
  return {
      {"resistor", "axial resistor",
       true, {cylinder(0.30, 0.48, 0.0, 0.24, 0.0, 90.0),
              lead(0.28, 0.035, 0.035, -0.38, 0.23),
              lead(0.28, 0.035, 0.035, 0.38, 0.23)}},
      {"potentiometer", "board-mount rotary potentiometer",
       true, {box(0.58, 0.32, 0.48, 0.0, 0.23),
              cylinder(0.21, 0.30, 0.0, 0.53),
              cylinder(0.34, 0.13, 0.0, 0.73),
              lead(0.045, 0.22, 0.055, -0.20, 0.05),
              lead(0.045, 0.22, 0.055, 0.0, 0.05),
              lead(0.045, 0.22, 0.055, 0.20, 0.05)}},
      {"ceramic_cap", "radial ceramic disc capacitor",
       true, {disc(0.50, 0.10, 0.0, 0.34),
              lead(0.045, 0.36, 0.045, -0.14, 0.12),
              lead(0.045, 0.36, 0.045, 0.14, 0.12)}},
      {"electrolytic_cap", "radial aluminum electrolytic capacitor",
       true, {cylinder(0.50, 0.56, 0.0, 0.36),
              cylinder(0.49, 0.025, 0.0, 0.65),
              lead(0.04, 0.16, 0.04, -0.10, 0.05),
              lead(0.04, 0.16, 0.04, 0.10, 0.05)}},
      {"inductor", "shielded radial inductor",
       true, {cylinder(0.54, 0.30, 0.0, 0.25),
              lead(0.04, 0.15, 0.04, -0.18, 0.05),
              lead(0.04, 0.15, 0.04, 0.18, 0.05)}},
      {"diode", "axial glass diode",
       true, {cylinder(0.26, 0.44, 0.0, 0.23, 0.0, 90.0),
              lead(0.28, 0.035, 0.035, -0.38, 0.23),
              lead(0.28, 0.035, 0.035, 0.38, 0.23)}},
      {"zener", "axial zener diode",
       true, {cylinder(0.26, 0.44, 0.0, 0.23, 0.0, 90.0),
              lead(0.28, 0.035, 0.035, -0.38, 0.23),
              lead(0.28, 0.035, 0.035, 0.38, 0.23)}},
      {"led", "through-hole LED",
       true, {cylinder(0.50, 0.09, 0.0, 0.22),
              sphere(0.45, 0.0, 0.41),
              lead(0.045, 0.24, 0.045, -0.09, 0.06),
              lead(0.055, 0.20, 0.055, 0.09, 0.08)}},
      {"npn", "TO-92 transistor",
       true, {sphere(0.50, 0.0, 0.34),
              lead(0.035, 0.26, 0.035, -0.18, 0.08),
              lead(0.035, 0.26, 0.035, 0.0, 0.08),
              lead(0.035, 0.26, 0.035, 0.18, 0.08)}},
      {"pnp", "TO-92 transistor",
       true, {sphere(0.50, 0.0, 0.34),
              lead(0.035, 0.26, 0.035, -0.18, 0.08),
              lead(0.035, 0.26, 0.035, 0.0, 0.08),
              lead(0.035, 0.26, 0.035, 0.18, 0.08)}},
      {"nmos", "TO-220 MOSFET",
       true, {box(0.52, 0.46, 0.16, 0.0, 0.32),
              box(0.46, 0.22, 0.06, 0.0, 0.64),
              lead(0.035, 0.25, 0.035, -0.17, 0.07),
              lead(0.035, 0.25, 0.035, 0.0, 0.07),
              lead(0.035, 0.25, 0.035, 0.17, 0.07)}},
      {"pmos", "TO-220 MOSFET",
       true, {box(0.52, 0.46, 0.16, 0.0, 0.32),
              box(0.46, 0.22, 0.06, 0.0, 0.64),
              lead(0.035, 0.25, 0.035, -0.17, 0.07),
              lead(0.035, 0.25, 0.035, 0.0, 0.07),
              lead(0.035, 0.25, 0.035, 0.17, 0.07)}},
      {"logic", "DIP logic IC", true, {box(0.80, 0.24, 0.46, 0.0, 0.24)}},
      {"opamp", "DIP-8 op-amp", true, {box(0.66, 0.24, 0.46, 0.0, 0.24)}},
      {"comparator", "DIP-8 comparator", true, {box(0.66, 0.24, 0.46, 0.0, 0.24)}},
      {"regulator", "tabbed voltage regulator",
       true, {box(0.48, 0.30, 0.20, 0.0, 0.25),
              box(0.52, 0.07, 0.34, 0.0, 0.16, -0.20)}},
      {"fuse", "cartridge fuse",
       true, {cylinder(0.26, 0.48, 0.0, 0.23, 0.0, 90.0),
              cylinder(0.29, 0.11, -0.25, 0.23, 0.0, 90.0),
              cylinder(0.29, 0.11, 0.25, 0.23, 0.0, 90.0)}},
      {"switch", "board-mount toggle switch",
       true, {box(0.58, 0.20, 0.42, 0.0, 0.18),
              box(0.08, 0.38, 0.08, 0.08, 0.43)}},
      {"relay", "sealed PCB relay", true, {box(0.66, 0.46, 0.54, 0.0, 0.30)}},
      {"connector", "pin header",
       true, {box(0.72, 0.18, 0.30, 0.0, 0.17),
              cylinder(0.052, 0.40, -0.27, 0.32),
              cylinder(0.052, 0.40, -0.09, 0.32),
              cylinder(0.052, 0.40, 0.09, 0.32),
              cylinder(0.052, 0.40, 0.27, 0.32)}},
      {"test_point", "loop test point",
       true, {cylinder(0.40, 0.035, 0.0, 0.04),
              cylinder(0.11, 0.38, 0.0, 0.24),
              cylinder(0.26, 0.055, 0.0, 0.43)}},
      {"sensor", "metal-can board sensor",
       true, {cylinder(0.48, 0.36, 0.0, 0.28),
              cylinder(0.26, 0.025, 0.0, 0.47)}},
      {"buzzer", "PCB piezo buzzer",
       true, {cylinder(0.58, 0.30, 0.0, 0.23),
              cylinder(0.11, 0.03, 0.0, 0.41)}},
      {"power", "two-position terminal block",
       true, {box(0.66, 0.42, 0.48, 0.0, 0.28),
              cylinder(0.17, 0.05, -0.18, 0.52),
              cylinder(0.17, 0.05, 0.18, 0.52)}},
      {"ground", "ground test stud",
       true, {cylinder(0.56, 0.045, 0.0, 0.04),
              cylinder(0.13, 0.32, 0.0, 0.22),
              cylinder(0.32, 0.04, 0.0, 0.40)}},
  };
}

}  // namespace

const std::vector<ComponentVisualRecipe>& component_visual_recipes() {
  static const std::vector<ComponentVisualRecipe> catalog = make_catalog();
  return catalog;
}

const ComponentVisualRecipe* find_component_visual_recipe(
    std::string_view component_id) {
  const auto& catalog = component_visual_recipes();
  const auto it = std::find_if(
      catalog.begin(), catalog.end(),
      [component_id](const ComponentVisualRecipe& recipe) {
        return recipe.component_id == component_id;
      });
  return it == catalog.end() ? nullptr : &*it;
}

}  // namespace formfactor
