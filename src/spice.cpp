#include "pcbtech/spice.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <set>
#include <sstream>
#include <type_traits>

namespace pcbtech {
namespace {

bool valid_token(const std::string& token) {
  if (token.empty()) return false;
  return std::all_of(token.begin(), token.end(), [](const unsigned char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || c == '_';
  });
}

std::string number(const double value) {
  std::ostringstream output;
  output << std::scientific << std::setprecision(17) << value;
  return output.str();
}

}  // namespace

bool SpiceDeckResult::valid() const { return errors.empty(); }

SpiceDeckResult export_spice_operating_point(
    const std::string& title, const std::vector<SpiceElement>& elements) {
  SpiceDeckResult result;
  if (title.empty() || title.find('\n') != std::string::npos ||
      title.find('\r') != std::string::npos) {
    result.errors.emplace_back("SPICE title must be non-empty and single-line");
  }
  if (elements.empty()) result.errors.emplace_back("SPICE deck requires at least one element");

  std::set<std::string> references;
  bool has_ground = false;
  std::vector<std::pair<std::string, std::string>> lines;

  for (const auto& element : elements) {
    std::visit([&](const auto& value) {
      using T = std::decay_t<decltype(value)>;
      const char prefix = std::is_same_v<T, SpiceResistor> ? 'R' : 'V';
      if (!valid_token(value.reference) || value.reference.front() != prefix) {
        result.errors.emplace_back(std::string("invalid SPICE reference: ") + value.reference);
      } else if (!references.insert(value.reference).second) {
        result.errors.emplace_back("duplicate SPICE reference: " + value.reference);
      }
      if (!valid_token(value.positive_node) || !valid_token(value.negative_node)) {
        result.errors.emplace_back("invalid SPICE node on: " + value.reference);
      }
      has_ground = has_ground || value.positive_node == "0" || value.negative_node == "0";

      const double quantity = [&]() {
        if constexpr (std::is_same_v<T, SpiceResistor>) {
          return value.resistance_ohms;
        } else {
          return value.voltage_volts;
        }
      }();
      if (!std::isfinite(quantity)) {
        result.errors.emplace_back("SPICE value must be finite: " + value.reference);
      }
      if constexpr (std::is_same_v<T, SpiceResistor>) {
        if (!(quantity > 0.0)) {
          result.errors.emplace_back("resistance must be greater than zero ohms: " + value.reference);
        }
      }
      lines.emplace_back(value.reference, value.reference + " " + value.positive_node + " " +
          value.negative_node + " " + number(quantity));
    }, element);
  }
  if (!has_ground) result.errors.emplace_back("SPICE deck requires reference node 0");
  if (!result.errors.empty()) return result;

  std::sort(lines.begin(), lines.end());
  std::ostringstream deck;
  deck << title << '\n';
  for (const auto& [reference, line] : lines) {
    (void)reference;
    deck << line << '\n';
  }
  deck << ".op\n.end\n";
  result.deck = deck.str();
  return result;
}

}  // namespace pcbtech
