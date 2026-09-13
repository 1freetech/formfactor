#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace pcbtech {

enum class LogicState { Low, High, Unknown };
enum class GateType { Not, And, Or };

struct DigitalGate {
  std::string name;
  GateType type{GateType::And};
  std::vector<std::string> inputs;
  std::string output;
  std::uint64_t delay_ns{};
};

struct DigitalEvent {
  std::uint64_t time_ns{};
  std::string net;
  LogicState state{LogicState::Unknown};
};

struct SimulationResult {
  std::map<std::string, LogicState> nets;
  std::vector<DigitalEvent> trace;
  std::vector<std::string> errors;
  [[nodiscard]] bool completed() const;
};

// Runs a deterministic, three-state, event-driven logic simulation. Gate
// delays are supplied by the caller; pcbtech does not infer timing data.
[[nodiscard]] SimulationResult simulate_digital(
    const std::vector<DigitalGate>& gates,
    const std::vector<DigitalEvent>& stimuli,
    std::size_t max_events = 10000);

}  // namespace pcbtech
