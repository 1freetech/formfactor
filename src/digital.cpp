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

LogicState evaluate(const DigitalGate& gate, const std::map<std::string, LogicState>& nets) {
  if (gate.type == GateType::Not) {
    const auto input = state_of(nets, gate.inputs.front());
    if (input == LogicState::Low) return LogicState::High;
    if (input == LogicState::High) return LogicState::Low;
    return LogicState::Unknown;
  }

  bool saw_unknown = false;
  for (const auto& input_name : gate.inputs) {
    const auto input = state_of(nets, input_name);
    if (gate.type == GateType::And && input == LogicState::Low) return LogicState::Low;
    if (gate.type == GateType::Or && input == LogicState::High) return LogicState::High;
    if (input == LogicState::Unknown) saw_unknown = true;
  }
  if (saw_unknown) return LogicState::Unknown;
  return gate.type == GateType::And ? LogicState::High : LogicState::Low;
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
    const std::size_t required_inputs = gate.type == GateType::Not ? 1 : 2;
    if (gate.inputs.size() < required_inputs || (gate.type == GateType::Not && gate.inputs.size() != 1)) {
      result.errors.emplace_back("gate has an invalid input count: " + gate.name);
    }
    if (gate.delay_ns == 0) result.errors.emplace_back("gate delay must be explicit and positive: " + gate.name);
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
      if (std::find(gate.inputs.begin(), gate.inputs.end(), queued.event.net) == gate.inputs.end()) continue;
      const auto output_state = evaluate(gate, result.nets);
      if (state_of(result.nets, gate.output) != output_state) {
        queue.push({{queued.event.time_ns + gate.delay_ns, gate.output, output_state}, sequence++});
      }
    }
  }
  return result;
}

}  // namespace formfactor
