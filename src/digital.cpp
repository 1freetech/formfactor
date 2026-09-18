#include "formfactor/digital.hpp"

#include <algorithm>
#include <queue>
#include <set>

namespace formfactor {
namespace {

struct QueuedEvent {
  DigitalEvent event;
  std::uint64_t sequence{};
};

struct EventOrder {
  bool operator()(const QueuedEvent& a, const QueuedEvent& b) const {
    if (a.event.time_ns != b.event.time_ns) return a.event.time_ns > b.event.time_ns;
    return a.sequence > b.sequence;
  }
};

LogicState state_of(const std::map<std::string, LogicState>& nets, const std::string& net) {
  const auto found = nets.find(net);
  return found == nets.end() ? LogicState::Unknown : found->second;
}

LogicState invert(const LogicState state) {
  if (state == LogicState::Low) return LogicState::High;
  if (state == LogicState::High) return LogicState::Low;
  return LogicState::Unknown;
}

LogicState evaluate_states(const GateType type, const std::vector<LogicState>& inputs) {
  if (type == GateType::Not) {
    if (inputs.size() != 1) return LogicState::Unknown;
    return invert(inputs.front());
  }

  if (type == GateType::And || type == GateType::Nand) {
    bool saw_unknown = false;
    for (const auto state : inputs) {
      if (state == LogicState::Low) {
        return type == GateType::And ? LogicState::Low : LogicState::High;
      }
      if (state == LogicState::Unknown) saw_unknown = true;
    }
    if (saw_unknown) return LogicState::Unknown;
    return type == GateType::And ? LogicState::High : LogicState::Low;
  }

  if (type == GateType::Or || type == GateType::Nor) {
    bool saw_unknown = false;
    for (const auto state : inputs) {
      if (state == LogicState::High) {
        return type == GateType::Or ? LogicState::High : LogicState::Low;
      }
      if (state == LogicState::Unknown) saw_unknown = true;
    }
    if (saw_unknown) return LogicState::Unknown;
    return type == GateType::Or ? LogicState::Low : LogicState::High;
  }

  bool saw_unknown = false;
  std::size_t high_count = 0;
  for (const auto state : inputs) {
    if (state == LogicState::Unknown) {
      saw_unknown = true;
    } else if (state == LogicState::High) {
      ++high_count;
    }
  }
  if (saw_unknown) return LogicState::Unknown;

  const bool odd = (high_count % 2) != 0;
  if (type == GateType::Xor) return odd ? LogicState::High : LogicState::Low;
  if (type == GateType::Xnor) return odd ? LogicState::Low : LogicState::High;
  return LogicState::Unknown;
}

LogicState evaluate(const DigitalGate& gate, const std::map<std::string, LogicState>& nets) {
  std::vector<LogicState> inputs;
  inputs.reserve(gate.inputs.size());
  for (const auto& input_name : gate.inputs) inputs.push_back(state_of(nets, input_name));
  return evaluate_states(gate.type, inputs);
}

bool valid_input_count(const GateType type, const std::size_t count) {
  if (type == GateType::Not) return count == 1;
  return count >= 2;
}

}  // namespace

bool SimulationResult::completed() const { return errors.empty(); }

SimulationResult simulate_digital(const std::vector<DigitalGate>& gates,
                                  const std::vector<DigitalEvent>& stimuli,
                                  const std::size_t max_events) {
  SimulationResult result;
  std::set<std::string> gate_names;
  std::set<std::string> driven_outputs;

  for (const auto& gate : gates) {
    if (gate.name.empty() || !gate_names.insert(gate.name).second) {
      result.errors.emplace_back("gate names must be non-empty and unique");
    }
    if (gate.output.empty() || !driven_outputs.insert(gate.output).second) {
      result.errors.emplace_back("gate outputs must be non-empty and uniquely driven");
    }
    if (!valid_input_count(gate.type, gate.inputs.size())) {
      result.errors.emplace_back("gate has an invalid input count: " + gate.name);
    }
    if (gate.delay_ns == 0) {
      result.errors.emplace_back("gate delay must be explicit and positive: " + gate.name);
    }
  }
  if (!result.errors.empty()) return result;

  std::priority_queue<QueuedEvent, std::vector<QueuedEvent>, EventOrder> queue;
  std::uint64_t sequence = 0;
  for (const auto& stimulus : stimuli) {
    if (stimulus.net.empty()) {
      result.errors.emplace_back("stimulus net is required");
    } else {
      queue.push({stimulus, sequence++});
    }
  }
  if (!result.errors.empty()) return result;

  std::size_t processed = 0;
  while (!queue.empty()) {
    if (processed >= max_events) {
      result.errors.emplace_back("event limit reached; circuit may oscillate");
      break;
    }
    const auto queued = queue.top();
    queue.pop();
    ++processed;

    if (state_of(result.nets, queued.event.net) == queued.event.state) continue;
    result.nets[queued.event.net] = queued.event.state;
    result.trace.push_back(queued.event);

    for (const auto& gate : gates) {
      if (std::find(gate.inputs.begin(), gate.inputs.end(), queued.event.net) == gate.inputs.end()) {
        continue;
      }
      const auto output_state = evaluate(gate, result.nets);
      if (state_of(result.nets, gate.output) != output_state) {
        queue.push({{queued.event.time_ns + gate.delay_ns, gate.output, output_state}, sequence++});
      }
    }
  }
  return result;
}

std::vector<TruthTableRow> generate_truth_table(const GateType type,
                                                 const std::size_t input_count) {
  if (!valid_input_count(type, input_count) || input_count > 16) return {};

  const std::size_t row_count = static_cast<std::size_t>(1) << input_count;
  std::vector<TruthTableRow> rows;
  rows.reserve(row_count);

  for (std::size_t pattern = 0; pattern < row_count; ++pattern) {
    TruthTableRow row;
    row.inputs.reserve(input_count);
    for (std::size_t input = 0; input < input_count; ++input) {
      const auto shift = input_count - input - 1;
      const bool high = ((pattern >> shift) & 1U) != 0;
      row.inputs.push_back(high ? LogicState::High : LogicState::Low);
    }
    row.output = evaluate_states(type, row.inputs);
    rows.push_back(std::move(row));
  }
  return rows;
}

}  // namespace formfactor
