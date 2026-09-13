#include "pcbtech/component.hpp"
#include <iostream>

int main() {
  const pcbtech::Component demo{
      "Example Semiconductor", "DEMO-001", "QFN-16", 16,
      5.5, 1.0, 150.0, true, pcbtech::AccuracyTier::Verified,
      {{"Authoritative datasheet", "https://example.invalid/datasheet", "A"}}};
  const auto result = pcbtech::validate(demo);
  std::cout << "pcbtech component gate: "
            << (result.export_allowed() ? "PASS" : "BLOCKED") << '\n';
  return result.export_allowed() ? 0 : 1;
}
