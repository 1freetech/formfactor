#include "formfactor/component.hpp"
#include <iostream>

int main() {
  const formfactor::Component demo{
      "Example Semiconductor", "DEMO-001", "QFN-16", 16,
      5.5, 1.0, 150.0, true, formfactor::AccuracyTier::Verified,
      {{"Authoritative datasheet", "https://example.invalid/datasheet", "A"}}};
  const auto result = formfactor::validate(demo);
  std::cout << "formfactor component gate: "
            << (result.export_allowed() ? "PASS" : "BLOCKED") << '\n';
  return result.export_allowed() ? 0 : 1;
}
