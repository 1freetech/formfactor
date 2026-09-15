# FormFactor

## Play FormFactor

No compiler or terminal setup is needed for the packaged builds.

- **Windows:** [Download the latest FormFactor Windows build](https://github.com/1freetech/formfactor/releases/download/latest/FormFactor-Windows-x64.zip), extract the ZIP, then double-click `FormFactor.exe`.
- **Linux:** [Download the latest FormFactor Linux build](https://github.com/1freetech/formfactor/releases/download/latest/FormFactor-Linux-x64.tar.gz), extract it, then open `FormFactor`.
- **Latest release page:** [FormFactor - Latest Playable Build](https://github.com/1freetech/formfactor/releases/tag/latest)

Every push to `main` now builds Windows and Linux packages, runs the implemented CTest suite on both platforms, and refreshes the `latest` release only after both builds pass.

formfactor is an open-source, physics-grounded PCB construction game and engineering simulator. The long-term goal is an interactive board builder whose designs are accepted only when engineering solvers and manufacturing checks support them.

Start with the downloads above if you only want to run the current graphical FreeLab prototype. Developers can use [Run FormFactor in VS Code](docs/GETTING_STARTED.md) for simple, numbered source-build instructions.

All setup help and future game screens follow the [simple instructions and component selection rules](docs/INTERACTION_RULES.md): visible buttons, one action per step, and accurate links between component names, schematic symbols, physical packages, and simulation models.

The game-facing design also follows the [Gameplay, Graphics, Presentation, and UX Benchmarks](docs/GAMEPLAY_UX_BENCHMARKS.md). That contract now combines lessons from highly rated games, open-world sandbox design, and creator platforms: an always-available FreeLab sandbox, multi-stage Engineering Contracts, reproducible checkpoints, context-preserving view switching, editable working templates, reusable scenario modules, rapid creator playtesting, form-factor preview, Lite-mode parity, accessibility, and presentation streaming that never weakens engineering truth. The detailed [GTA and Open-World Design Research](docs/GTA_OPEN_WORLD_RESEARCH.md) and [Fortnite and Roblox Creator-System Research](docs/FORTNITE_ROBLOX_CREATOR_RESEARCH.md) record the source-specific lessons and license guardrails. Proprietary game assets, missions, branding, platform code, and reverse-engineered GTA source are not copied into FormFactor.

## Accuracy contract

- The renderer never decides whether a circuit works.
- Every component carries provenance and an accuracy tier: `verified`, `partial`, or `visual-only`.
- `verified` requires authoritative pinout, electrical limits, package, thermal data, and a simulation model.
- Missing data remains explicitly unknown. FormFactor must not invent electrical values.
- Manufacturing export is blocked unless all required validation gates pass.

## First vertical slice

The initial C++ core validates component records before they can enter a simulated design. It deliberately rejects unverified or physically invalid parts. Its topology-only electrical-rule checker also rejects duplicate net names, pins assigned to multiple nets, and conflicting power or digital outputs. Dangling nets are reported as warnings because intent cannot be inferred safely.

The catalogue links a stable part identity to revisioned symbol, footprint, physical-model, simulation-model, and explicit pin-mapping records. Its [initial quantity-property layer](docs/CATALOG_QUANTITY_PROPERTIES.md) attaches sourced exact-SI values, qualifiers, and optional conditions, then emits deterministic replay records. Missing properties stay absent; exact numeric filtering and typed property schemas now fail closed on unsupported component-family use. The family registry states implemented support only and does not claim an excluded property is physically impossible; source authentication and graphical component cards remain pending.

The deterministic digital-event slice supports explicit-delay NOT, AND, and OR gates with `low`, `high`, and `unknown` states. Timing must be supplied by the caller; absent component timing is never fabricated.

The initial SPICE adapter exports a deterministic ngspice-compatible `.op` deck for explicitly valued resistors and independent DC voltage sources. It enforces finite SI values, positive resistance, reference node `0`, unique references, and injection-safe identifiers. Solver execution and model-library support remain future work.

The initial multilayer stackup model records top-to-bottom copper and dielectric layers with explicit micrometre units and authoritative material sources. It validates ordering, identity, thickness, and caller-supplied dielectric properties, emits a deterministic record, and blocks fabrication export when required data is invalid or missing. It does not yet calculate controlled impedance or assume fabrication tolerances.

The initial layout model records pads, vias, and straight trace segments in exact integer nanometres. Caller-supplied, sourced manufacturing rules govern minimum trace width, via drill, annular ring, and copper clearance; invalid geometry or missing provenance blocks fabrication export. Curved traces, polygons, and KiCad serialization remain pending.

SPICE execution evidence can be stored in a deterministic, length-delimited replay record containing the exact input deck, solver name and version, portable exit status, and raw output streams. The record proves only that the supplied evidence is complete enough to preserve; FormFactor does not yet invoke a solver or interpret the output as an electrically valid result.

Circular pad and via clearance and straight-trace-to-circular-copper clearance are checked exactly across overlapping copper layers using 128-bit distance comparisons inside a declared safe numeric domain. Different nets must meet the sourced clearance; same-net copper is exempt. Trace-to-trace and polygon clearance remain pending and are not approximated.

Every trace also carries an explicit current load and sourced current limit in integer microamperes. Loads at the limit pass; overloads, missing values, and missing provenance block fabrication export. formfactor does not yet calculate ampacity or current density because those require validated copper-thickness, temperature-rise, and thermal-boundary inputs.

Controlled-impedance requirements can be recorded for single-ended traces and differential pairs in exact integer milliohms. Constraints require trace references, positive target/tolerance intervals, and authoritative sources. The current slice validates requirements only; it does not claim a routed structure meets them without a validated field solver.

Return-path requirements bind signal traces to known reference nets with explicit maximum discontinuity lengths in integer nanometres and authoritative sources. The validator prepares deterministic solver input but does not claim physical continuity until reference-plane geometry is modeled and evaluated.

PDN requirements use the standard target-impedance relationship `Z_target = delta-V / delta-I`. Ripple and load step are supplied in microvolts and microamps, then stored as an exact reduced rational impedance in ohms alongside bandwidth and provenance. No decimal precision, decoupling response, or pass result is fabricated.

Decoupling requirements bind known capacitor references to power nets with explicit minimum capacitance and voltage rating, maximum ESR and ESL, placement distance, and authoritative provenance. The validator emits deterministic solver input only; it does not infer bias derating, frequency response, mounting inductance, or PDN compliance.

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/formfactor_validate
```

The engineering core and CTest suite configure without graphical dependencies.
The dedicated policy-signature verifier requires OpenSSL Crypto 1.1.1 or newer;
use `-DFORMFACTOR_BUILD_SIGNATURE_VERIFIER=OFF` only when intentionally
building without that cryptographic gate. FreeLab is built automatically when `pkg-config` and the SDL2 development files
are available; use `-DFORMFACTOR_BUILD_FREELAB=OFF` to disable that optional
target explicitly.

For a complete CMake build and CTest run on a machine where CMake is not yet
installed, use the reproducible local bootstrap command below. It installs the
pinned tool under the ignored `build/` directory without changing system files.

```bash
sh scripts/cmake_validate.sh
```

If CMake is unavailable, run the direct-compiler fallback. The complete fallback requires the OpenSSL Crypto headers and library:

```bash
sh scripts/validate.sh
```

## Engineering pipeline

formfactor follows a straightforward engineering flow. Each stage must produce trustworthy information before the next stage can rely on it.

**Component data → Validation → Circuit → Simulation → PCB checks → Visualization → Gated export**

1. **Start with known components.** Load versioned component records and keep the source for every important electrical, thermal, and physical property.
2. **Check the component data.** Confirm package, pinout, ratings, timing, thermal limits, and available simulation models. Missing information stays unknown instead of being guessed.
3. **Build and check the circuit.** Connect components into nets, then catch conflicts such as duplicate names, impossible connections, or incompatible outputs.
4. **Simulate electrical behavior.** Use deterministic digital simulation for supported logic and SPICE-backed analysis for supported analog and power behavior.
5. **Check the PCB design.** Validate layout rules and, when the required solvers and source data exist, evaluate signal integrity, power integrity, thermal behavior, and manufacturability.
6. **Show the results.** The game engine presents solver output so users can see what passed, what failed, and what is still unknown. The renderer never decides engineering truth.
7. **Export only validated work.** KiCad and manufacturing artifacts become eligible for export only after every required implemented validation gate passes.

## Pinned policy-key trust evaluation

CR-020 can evaluate an exact publisher/key binding against a caller-supplied, versioned trust root at a caller-supplied UTC instant. The root must contain valid CR-017 key material, a nonzero version, a fixed-format UTC expiry, and unambiguous publisher/key identities. Unknown or removed keys are not trusted, expired roots cannot authorize keys, malformed inputs fail closed, and deterministic records omit raw public-key bytes.

This gate does not authenticate or distribute the trust root, preserve a previously trusted version, prevent rollback, verify threshold rotation, or authorize catalogue export. Those stateful and signed-root dependencies remain required before a caller-supplied root can become authoritative.

## Root update sequence precheck

CR-021 rejects rollback, repeated-version, skipped-version, different-root, expired-candidate, malformed, and overflowing key-root transitions. Only an exact N-to-N+1 transition can produce `Sequential`, and deterministic records state that signature verification and persistence were not performed.

A sequential result is not a trusted root update. FormFactor still requires old-root and new-root signature thresholds, unique signer counting, authenticated distribution, durable trusted-state persistence, and catalogue-export integration.

---

## Download the latest playable build

- **Windows:** [FormFactor-Windows-x64.zip](https://github.com/1freetech/formfactor/releases/download/latest/FormFactor-Windows-x64.zip)
- **Linux:** [FormFactor-Linux-x64.tar.gz](https://github.com/1freetech/formfactor/releases/download/latest/FormFactor-Linux-x64.tar.gz)
- **Release page:** [Latest playable release](https://github.com/1freetech/formfactor/releases/tag/latest)

These links stay the same while GitHub replaces the files with the newest tested build from `main`.
