#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace formfactor {

enum class LogicState { Low, High, Unknown };
enum class GateType { Not, And, Or, Nand, Nor, Xor, Xnor };

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

struct TruthTableRow {
  std::vector<LogicState> inputs;
  LogicState output{LogicState::Unknown};
};

// Runs a deterministic, three-state, event-driven logic simulation. Gate
// delays are supplied by the caller; FormFactor does not infer timing data.
[[nodiscard]] SimulationResult simulate_digital(
    const std::vector<DigitalGate>& gates,
    const std::vector<DigitalEvent>& stimuli,
    std::size_t max_events = 10000);

// Generates a complete Low/High truth table for one supported gate family.
// NOT requires one input. All other current gate families require at least two
// inputs. Input counts above 16 are rejected to avoid accidental huge tables.
[[nodiscard]] std::vector<TruthTableRow> generate_truth_table(
    GateType type, std::size_t input_count);

}  // namespace formfactor
