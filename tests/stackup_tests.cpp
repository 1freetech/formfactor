#include "pcbtech/stackup.hpp"

#include <cassert>
#include <limits>
#include <vector>

int main() {
  using pcbtech::StackupLayer;
  using pcbtech::StackupLayerKind;
  const std::vector<StackupLayer> two_layer{
      {"F_Cu", StackupLayerKind::copper, 35.0, "Cu", "fabricator-datasheet", {}, {}},
      {"Core", StackupLayerKind::dielectric, 1500.0, "laminate", "laminate-datasheet", 4.2, 0.02},
      {"B_Cu", StackupLayerKind::copper, 35.0, "Cu", "fabricator-datasheet", {}, {}}};
  const auto first = pcbtech::validate_stackup("explicit-2-layer", two_layer);
  const auto replay = pcbtech::validate_stackup("explicit-2-layer", two_layer);
  assert(first.valid());
  assert(first.fabrication_export_allowed());
  assert(first.canonical_record == replay.canonical_record);
  assert(first.canonical_record.find("3.50000000000000000e+01 um") != std::string::npos);

  auto invalid = two_layer;
  invalid[1].thickness_micrometres = 0.0;
  assert(!pcbtech::validate_stackup("zero-thickness", invalid).valid());
  invalid = two_layer;
  invalid[1].thickness_micrometres = std::numeric_limits<double>::infinity();
  assert(!pcbtech::validate_stackup("infinite-thickness", invalid).valid());
  invalid = two_layer;
  invalid[1].authoritative_source.clear();
  assert(!pcbtech::validate_stackup("missing-source", invalid).fabrication_export_allowed());
  invalid = two_layer;
  invalid[1].relative_permittivity = 0.0;
  assert(!pcbtech::validate_stackup("invalid-er", invalid).valid());
  invalid = two_layer;
  invalid[1].loss_tangent = 0.0;  // valid physical boundary
  assert(pcbtech::validate_stackup("zero-loss-boundary", invalid).valid());
  invalid = two_layer;
  invalid[2].name = "F_Cu";
  assert(!pcbtech::validate_stackup("duplicate-name", invalid).valid());
  invalid = two_layer;
  invalid.insert(invalid.begin() + 1, two_layer.front());
  assert(!pcbtech::validate_stackup("bad-order", invalid).valid());
  assert(!pcbtech::validate_stackup("bad\nname", two_layer).valid());
}
