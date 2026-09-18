#include "formfactor/kicad_import.hpp"

#include <cassert>
#include <string>

int main() {
  // MIT-licensed reference values mirror representative records from
  // embeddedalpha/PCB-Design Project X and are attributed in
  // THIRD_PARTY_NOTICES.md.
  const std::string schematic =
      "EESchema Schematic File Version 4\n"
      "$Comp\n"
      "L Device:R R1\n"
      "U 1 1 00000001\n"
      "P 1000 1000\n"
      "F 0 \"R1\" H 1000 1050 50  0000 C CNN\n"
      "F 1 \"1.1K\" H 1000 950 50  0000 C CNN\n"
      "F 2 \"Resistor_SMD:R_0402_1005Metric\" H 1000 1000 50  0001 C CNN\n"
      "$EndComp\n"
      "$Comp\n"
      "L Device:C_Small C3\n"
      "U 1 1 00000002\n"
      "P 2000 1000\n"
      "F 0 \"C3\" H 2000 1050 50  0000 C CNN\n"
      "F 1 \"100N\" H 2000 950 50  0000 C CNN\n"
      "F 2 \"Capacitor_SMD:C_0201_0603Metric\" H 2000 1000 50  0001 C CNN\n"
      "$EndComp\n";

  const auto parsed = formfactor::parse_legacy_kicad_schematic(schematic);
  assert(parsed.valid());
  assert(parsed.components.size() == 2);
  assert(parsed.components[0].reference == "R1");
  assert(parsed.components[0].symbol_library_id == "Device:R");
  assert(parsed.components[0].value == "1.1K");
  assert(parsed.components[0].footprint_library_id == "Resistor_SMD:R_0402_1005Metric");
  assert(parsed.components[1].reference == "C3");
  assert(parsed.components[1].symbol_library_id == "Device:C_Small");
  assert(parsed.components[1].footprint_library_id == "Capacitor_SMD:C_0201_0603Metric");

  const auto duplicate = formfactor::parse_legacy_kicad_schematic(
      "$Comp\nL Device:R R1\n$EndComp\n"
      "$Comp\nL Device:C R1\n$EndComp\n");
  assert(!duplicate.valid());

  const auto unterminated = formfactor::parse_legacy_kicad_schematic(
      "$Comp\nL Device:R R2\n");
  assert(!unterminated.valid());

  const auto partial = formfactor::parse_legacy_kicad_schematic(
      "$Comp\nL Device:R R3\nF 1 \"10K\"\n$EndComp\n");
  assert(partial.valid());
  assert(partial.components.size() == 1);
  assert(partial.components[0].footprint_library_id.empty());
}
