#include "formfactor/digital.hpp"

#include <cassert>

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
