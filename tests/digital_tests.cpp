#include "formfactor/digital.hpp"

#include <cassert>
#include <vector>

int main() {
  using formfactor::DigitalEvent;
  using formfactor::DigitalGate;
  using formfactor::GateType;
  using formfactor::LogicState;

  const std::vector<DigitalGate> inverter_chain{
      {"INV1", GateType::Not, {"A"}, "B", 5},
      {"INV2", GateType::Not, {"B"}, "C", 7}};
  const auto chain = formfactor::simulate_digital(inverter_chain, {{0, "A", LogicState::High}});
  assert(chain.completed());
  assert(chain.nets.at("B") == LogicState::Low);
  assert(chain.nets.at("C") == LogicState::High);
  assert(chain.trace.back().time_ns == 12);

  const std::vector<DigitalGate> and_gate{{"AND1", GateType::And, {"A", "B"}, "Y", 3}};
  const auto dominant_low = formfactor::simulate_digital(
      and_gate, {{0, "A", LogicState::Unknown}, {0, "B", LogicState::Low}});
  assert(dominant_low.completed());
  assert(dominant_low.nets.at("Y") == LogicState::Low);

  const auto unknown_output = formfactor::simulate_digital(
      and_gate, {{0, "A", LogicState::Unknown}, {0, "B", LogicState::High}});
  assert(unknown_output.completed());
  assert(unknown_output.nets.find("Y") == unknown_output.nets.end());

  const std::vector<DigitalGate> nand_gate{{"N1", GateType::Nand, {"A", "B"}, "Y", 2}};
  const auto nand_result = formfactor::simulate_digital(
      nand_gate, {{0, "A", LogicState::High}, {0, "B", LogicState::High}});
  assert(nand_result.completed());
  assert(nand_result.nets.at("Y") == LogicState::Low);

  const std::vector<DigitalGate> nor_gate{{"NOR1", GateType::Nor, {"A", "B"}, "Y", 2}};
  const auto nor_result = formfactor::simulate_digital(
      nor_gate, {{0, "A", LogicState::Low}, {0, "B", LogicState::Low}});
  assert(nor_result.completed());
  assert(nor_result.nets.at("Y") == LogicState::High);

  const std::vector<DigitalGate> xor_gate{{"X1", GateType::Xor, {"A", "B"}, "Y", 2}};
  const auto xor_result = formfactor::simulate_digital(
      xor_gate, {{0, "A", LogicState::High}, {0, "B", LogicState::Low}});
  assert(xor_result.completed());
  assert(xor_result.nets.at("Y") == LogicState::High);

  const std::vector<DigitalGate> xnor_gate{{"XN1", GateType::Xnor, {"A", "B"}, "Y", 2}};
  const auto xnor_result = formfactor::simulate_digital(
      xnor_gate, {{0, "A", LogicState::High}, {0, "B", LogicState::High}});
  assert(xnor_result.completed());
  assert(xnor_result.nets.at("Y") == LogicState::High);

  const auto xor_unknown = formfactor::simulate_digital(
      xor_gate, {{0, "A", LogicState::Unknown}, {0, "B", LogicState::High}});
  assert(xor_unknown.completed());
  assert(xor_unknown.nets.find("Y") == xor_unknown.nets.end());

  const auto not_table = formfactor::generate_truth_table(GateType::Not, 1);
  assert(not_table.size() == 2);
  assert(not_table[0].inputs[0] == LogicState::Low);
  assert(not_table[0].output == LogicState::High);
  assert(not_table[1].inputs[0] == LogicState::High);
  assert(not_table[1].output == LogicState::Low);

  const auto xor_table = formfactor::generate_truth_table(GateType::Xor, 2);
  assert(xor_table.size() == 4);
  assert(xor_table[0].output == LogicState::Low);
  assert(xor_table[1].output == LogicState::High);
  assert(xor_table[2].output == LogicState::High);
  assert(xor_table[3].output == LogicState::Low);

  const auto xnor_table = formfactor::generate_truth_table(GateType::Xnor, 2);
  assert(xnor_table.size() == 4);
  assert(xnor_table[0].output == LogicState::High);
  assert(xnor_table[3].output == LogicState::High);

  const auto invalid_not_table = formfactor::generate_truth_table(GateType::Not, 2);
  assert(invalid_not_table.empty());
  const auto huge_table = formfactor::generate_truth_table(GateType::And, 17);
  assert(huge_table.empty());

  const auto missing_delay = formfactor::simulate_digital(
      {{"INV", GateType::Not, {"A"}, "Y", 0}}, {{0, "A", LogicState::Low}});
  assert(!missing_delay.completed());

  const auto duplicate_driver = formfactor::simulate_digital(
      {{"A1", GateType::And, {"A", "B"}, "Y", 1},
       {"O1", GateType::Or, {"C", "D"}, "Y", 1}}, {});
  assert(!duplicate_driver.completed());

  const auto bad_not = formfactor::simulate_digital(
      {{"INV", GateType::Not, {"A", "B"}, "Y", 1}}, {});
  assert(!bad_not.completed());
}
