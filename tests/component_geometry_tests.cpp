#include "formfactor/component_geometry.hpp"

#include <cassert>
#include <string_view>
#include <unordered_set>

int main() {
  using namespace formfactor;

  const auto& catalog = component_visual_recipes();
  assert(catalog.size() == 25U);

  std::unordered_set<std::string_view> ids;
  for (const auto& recipe : catalog) {
    assert(!recipe.component_id.empty());
    assert(!recipe.package_style.empty());
    assert(recipe.visual_only);
    assert(!recipe.primitives.empty());
    assert(ids.insert(recipe.component_id).second);
  }

  for (const auto id : {"resistor", "potentiometer", "ceramic_cap",
                        "electrolytic_cap", "inductor", "diode", "zener",
                        "led", "npn", "pnp", "nmos", "pmos", "logic",
                        "opamp", "comparator", "regulator", "fuse",
                        "switch", "relay", "connector", "test_point",
                        "sensor", "buzzer", "power", "ground"}) {
    assert(find_component_visual_recipe(id) != nullptr);
  }

  assert(find_component_visual_recipe("not-a-component") == nullptr);
  return 0;
}
