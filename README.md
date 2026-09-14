# pcbtech

pcbtech is an open-source, physics-grounded PCB construction game and engineering simulator. The long-term goal is an interactive board builder whose designs are accepted only when engineering solvers and manufacturing checks support them.

Start with [Run pcbtech in VS Code](docs/GETTING_STARTED.md) for simple, numbered setup instructions. The current build runs in a terminal; the graphical workbench is still planned.

All setup help and future game screens follow the [simple instructions and component selection rules](docs/INTERACTION_RULES.md): visible buttons, one action per step, and accurate links between component names, schematic symbols, physical packages, and simulation models.

## Accuracy contract

- The renderer never decides whether a circuit works.
- Every component carries provenance and an accuracy tier: `verified`, `partial`, or `visual-only`.
- `verified` requires authoritative pinout, electrical limits, package, thermal data, and a simulation model.
- Missing data remains explicitly unknown. PCBTech must not invent electrical values.
- Manufacturing export is blocked unless all required validation gates pass.

## First vertical slice

The initial C++ core validates component records before they can enter a simulated design. It deliberately rejects unverified or physically invalid parts. Its topology-only electrical-rule checker also rejects duplicate net names, pins assigned to multiple nets, and conflicting power or digital outputs. Dangling nets are reported as warnings because intent cannot be inferred safely.

The catalogue links a stable part identity to revisioned symbol, footprint, physical-model, simulation-model, and explicit pin-mapping records. Its [initial quantity-property layer](docs/CATALOG_QUANTITY_PROPERTIES.md) attaches sourced exact-SI values, qualifiers, and optional conditions, then emits deterministic replay records. Missing properties stay absent; typed property schemas, numeric filtering, source authentication, and graphical component cards remain pending.

The deterministic digital-event slice supports explicit-delay NOT, AND, and OR gates with `low`, `high`, and `unknown` states. Timing must be supplied by the caller; absent component timing is never fabricated.

The initial SPICE adapter exports a deterministic ngspice-compatible `.op` deck for explicitly valued resistors and independent DC voltage sources. It enforces finite SI values, positive resistance, reference node `0`, unique references, and injection-safe identifiers. Solver execution and model-library support remain future work.

The initial multilayer stackup model records top-to-bottom copper and dielectric layers with explicit micrometre units and authoritative material sources. It validates ordering, identity, thickness, and caller-supplied dielectric properties, emits a deterministic record, and blocks fabrication export when required data is invalid or missing. It does not yet calculate controlled impedance or assume fabrication tolerances.

The initial layout model records pads, vias, and straight trace segments in exact integer nanometres. Caller-supplied, sourced manufacturing rules govern minimum trace width, via drill, annular ring, and copper clearance; invalid geometry or missing provenance blocks fabrication export. Curved traces, polygons, and KiCad serialization remain pending.

Circular pad and via clearance and straight-trace-to-circular-copper clearance are checked exactly across overlapping copper layers using 128-bit distance comparisons inside a declared safe numeric domain. Different nets must meet the sourced clearance; same-net copper is exempt. Trace-to-trace and polygon clearance remain pending and are not approximated.

Every trace also carries an explicit current load and sourced current limit in integer microamperes. Loads at the limit pass; overloads, missing values, and missing provenance block fabrication export. pcbtech does not yet calculate ampacity or current density because those require validated copper-thickness, temperature-rise, and thermal-boundary inputs.

Controlled-impedance requirements can be recorded for single-ended traces and differential pairs in exact integer milliohms. Constraints require trace references, positive target/tolerance intervals, and authoritative sources. The current slice validates requirements only; it does not claim a routed structure meets them without a validated field solver.

Return-path requirements bind signal traces to known reference nets with explicit maximum discontinuity lengths in integer nanometres and authoritative sources. The validator prepares deterministic solver input but does not claim physical continuity until reference-plane geometry is modeled and evaluated.

PDN requirements use the standard target-impedance relationship `Z_target = delta-V / delta-I`. Ripple and load step are supplied in microvolts and microamps, then stored as an exact reduced rational impedance in ohms alongside bandwidth and provenance. No decimal precision, decoupling response, or pass result is fabricated.

Decoupling requirements bind known capacitor references to power nets with explicit minimum capacitance and voltage rating, maximum ESR and ESL, placement distance, and authoritative provenance. The validator emits deterministic solver input only; it does not infer bias derating, frequency response, mounting inductance, or PDN compliance.

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/pcbtech_validate
```

If CMake is unavailable, run the dependency-free fallback:

```bash
sh scripts/validate.sh
```

## Engineering pipeline

pcbtech follows a straightforward engineering flow. Each stage must produce trustworthy information before the next stage can rely on it.

**Component data → Validation → Circuit → Simulation → PCB checks → Visualization → Gated export**

1. **Start with known components.** Load versioned component records and keep the source for every important electrical, thermal, and physical property.
2. **Check the component data.** Confirm package, pinout, ratings, timing, thermal limits, and available simulation models. Missing information stays unknown instead of being guessed.
3. **Build and check the circuit.** Connect components into nets, then catch conflicts such as duplicate names, impossible connections, or incompatible outputs.
4. **Simulate electrical behavior.** Use deterministic digital simulation for supported logic and SPICE-backed analysis for supported analog and power behavior.
5. **Check the PCB design.** Validate layout rules and, when the required solvers and source data exist, evaluate signal integrity, power integrity, thermal behavior, and manufacturability.
6. **Show the results.** The game engine presents solver output so users can see what passed, what failed, and what is still unknown. The renderer never decides engineering truth.
7. **Export only validated work.** KiCad and manufacturing artifacts become eligible for export only after every required implemented validation gate passes.
