#include "formfactor/stackup.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <set>
#include <sstream>

namespace formfactor {
namespace {

bool single_line(const std::string& value) {
  return !value.empty() && value.find('\n') == std::string::npos &&
         value.find('\r') == std::string::npos;
}

std::string number(const double value) {
  std::ostringstream output;
  output << std::scientific << std::setprecision(17) << value;
  return output.str();
}

}  // namespace

bool StackupValidation::valid() const { return errors.empty(); }

bool StackupValidation::fabrication_export_allowed() const {
  return valid() && !canonical_record.empty();
}

StackupValidation validate_stackup(const std::string& name,
                                   const std::vector<StackupLayer>& layers) {
  StackupValidation result;
  if (!single_line(name)) result.errors.emplace_back("stackup name must be non-empty and single-line");
  if (layers.size() < 3U) result.errors.emplace_back("stackup requires at least two copper layers and one dielectric");

  std::set<std::string> names;
  std::size_t copper_count = 0;
  for (std::size_t index = 0; index < layers.size(); ++index) {
    const auto& layer = layers[index];
    if (!single_line(layer.name)) result.errors.emplace_back("layer name must be non-empty and single-line");
    else if (!names.insert(layer.name).second) result.errors.emplace_back("duplicate layer name: " + layer.name);
    if (!single_line(layer.material)) result.errors.emplace_back("material must be identified: " + layer.name);
    if (!single_line(layer.authoritative_source)) {
      result.errors.emplace_back("authoritative material source required: " + layer.name);
    }
    if (!std::isfinite(layer.thickness_micrometres) || !(layer.thickness_micrometres > 0.0)) {
      result.errors.emplace_back("layer thickness must be finite and greater than zero micrometres: " + layer.name);
    }
    if (layer.kind == StackupLayerKind::copper) {
      ++copper_count;
      if (layer.relative_permittivity || layer.loss_tangent) {
        result.errors.emplace_back("dielectric properties are not valid on copper layer: " + layer.name);
      }
    } else {
      if (layer.relative_permittivity &&
          (!std::isfinite(*layer.relative_permittivity) || !(*layer.relative_permittivity > 0.0))) {
        result.errors.emplace_back("relative permittivity must be finite and greater than zero: " + layer.name);
      }
      if (layer.loss_tangent &&
          (!std::isfinite(*layer.loss_tangent) || *layer.loss_tangent < 0.0)) {
        result.errors.emplace_back("loss tangent must be finite and non-negative: " + layer.name);
      }
    }
    if (index > 0 && layer.kind == layers[index - 1].kind) {
      result.errors.emplace_back("copper and dielectric layers must alternate");
    }
  }
  if (copper_count < 2U) result.errors.emplace_back("stackup requires at least two copper layers");
  if (!layers.empty() && (layers.front().kind != StackupLayerKind::copper ||
                          layers.back().kind != StackupLayerKind::copper)) {
    result.errors.emplace_back("outer stackup layers must be copper");
  }
  if (!result.errors.empty()) return result;

  std::ostringstream record;
  record << "formfactor-stackup-v1 " << name << '\n';
  for (const auto& layer : layers) {
    record << (layer.kind == StackupLayerKind::copper ? "copper" : "dielectric")
           << ' ' << layer.name << ' ' << number(layer.thickness_micrometres)
           << " um " << layer.material << " source=" << layer.authoritative_source;
    if (layer.relative_permittivity) record << " er=" << number(*layer.relative_permittivity);
    if (layer.loss_tangent) record << " tand=" << number(*layer.loss_tangent);
    record << '\n';
  }
  result.canonical_record = record.str();
  return result;
}

}  // namespace formfactor
