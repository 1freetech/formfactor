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
