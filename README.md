# pcbtech

pcbtech is an open-source, physics-grounded PCB construction game and engineering simulator. The long-term goal is an interactive board builder whose designs are accepted only when engineering solvers and manufacturing checks support them.

## Accuracy contract

- The renderer never decides whether a circuit works.
- Every component carries provenance and an accuracy tier: `verified`, `partial`, or `visual-only`.
- `verified` requires authoritative pinout, electrical limits, package, thermal data, and a simulation model.
- Missing data remains explicitly unknown. PCBTech must not invent electrical values.
- Manufacturing export is blocked unless all required validation gates pass.

## First vertical slice

The initial C++ core validates component records before they can enter a simulated design. It deliberately rejects unverified or physically invalid parts. Its topology-only electrical-rule checker also rejects duplicate net names, pins assigned to multiple nets, and conflicting power or digital outputs. Dangling nets are reported as warnings because intent cannot be inferred safely.

The deterministic digital-event slice supports explicit-delay NOT, AND, and OR gates with `low`, `high`, and `unknown` states. Timing must be supplied by the caller; absent component timing is never fabricated.

The initial SPICE adapter exports a deterministic ngspice-compatible `.op` deck for explicitly valued resistors and independent DC voltage sources. It enforces finite SI values, positive resistance, reference node `0`, unique references, and injection-safe identifiers. Solver execution and model-library support remain future work.

The initial multilayer stackup model records top-to-bottom copper and dielectric layers with explicit micrometre units and authoritative material sources. It validates ordering, identity, thickness, and caller-supplied dielectric properties, emits a deterministic record, and blocks fabrication export when required data is invalid or missing. It does not yet calculate controlled impedance or assume fabrication tolerances.

The initial layout model records pads, vias, and straight trace segments in exact integer nanometres. Caller-supplied, sourced manufacturing rules govern minimum trace width, via drill, annular ring, and copper clearance; invalid geometry or missing provenance blocks fabrication export. Curved traces, polygons, and KiCad serialization remain pending.

Circular pad and via clearance is checked exactly across overlapping copper layers using 128-bit squared distances inside a declared safe numeric domain. Different nets must meet the sourced clearance; same-net copper is exempt. Trace and polygon clearance remain pending and are not approximated.

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

1. Import versioned component data with source provenance.
2. Validate packages, pins, ratings, timing, thermal data, and models.
3. Assemble connectivity and reject impossible nets.
4. Run digital event simulation and SPICE-backed analog/power analysis.
5. Run layout, signal-integrity, thermal, and manufacturability checks.
6. Visualize solver output in the game engine.
7. Export KiCad and manufacturing artifacts only after required gates pass.
