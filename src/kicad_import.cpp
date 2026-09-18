#include "formfactor/kicad_import.hpp"

#include <algorithm>
#include <cctype>
#include <set>
#include <sstream>

namespace formfactor {
namespace {

std::string trim(std::string value) {
  while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) {
    value.erase(value.begin());
  }
  while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) {
    value.pop_back();
  }
  return value;
}

std::string quoted_value(const std::string& line) {
  const auto first = line.find('"');
  if (first == std::string::npos) return {};
  const auto second = line.find('"', first + 1);
  if (second == std::string::npos) return {};
  return line.substr(first + 1, second - first - 1);
}

bool safe_reference(const std::string& value) {
  if (value.empty()) return false;
  return std::all_of(value.begin(), value.end(), [](const unsigned char c) {
    return std::isalnum(c) != 0 || c == '_' || c == '-' || c == '.';
  });
}

}  // namespace

bool KicadLegacyImport::valid() const { return errors.empty(); }

KicadLegacyImport parse_legacy_kicad_schematic(const std::string_view source) {
  KicadLegacyImport result;
  std::istringstream input{std::string(source)};
  std::string line;
  bool in_component = false;
  KicadLegacyComponent current;
  std::set<std::string> references;
  std::size_t line_number = 0;

  auto finish_component = [&]() {
    if (current.reference.empty() || current.symbol_library_id.empty()) {
      result.errors.emplace_back("component is missing legacy L symbol/reference fields");
    } else if (!safe_reference(current.reference)) {
      result.errors.emplace_back("component reference is not a safe token: " + current.reference);
    } else if (!references.insert(current.reference).second) {
      result.errors.emplace_back("duplicate component reference: " + current.reference);
    } else {
      result.components.push_back(current);
    }
    current = {};
  };

  while (std::getline(input, line)) {
    ++line_number;
    const auto text = trim(line);
    if (text == "$Comp") {
      if (in_component) {
        result.errors.emplace_back("nested $Comp at line " + std::to_string(line_number));
        current = {};
      }
      in_component = true;
      continue;
    }
    if (text == "$EndComp") {
      if (!in_component) {
        result.errors.emplace_back("unexpected $EndComp at line " + std::to_string(line_number));
        continue;
      }
      finish_component();
      in_component = false;
      continue;
    }
    if (!in_component) continue;

    if (text.rfind("L ", 0) == 0) {
      std::istringstream fields{text.substr(2)};
      fields >> current.symbol_library_id >> current.reference;
      continue;
    }
    if (text.rfind("F 1 ", 0) == 0) {
      current.value = quoted_value(text);
      continue;
    }
    if (text.rfind("F 2 ", 0) == 0) {
      current.footprint_library_id = quoted_value(text);
      continue;
    }
  }

  if (in_component) {
    result.errors.emplace_back("unterminated $Comp block");
  }

  return result;
}

}  // namespace formfactor
