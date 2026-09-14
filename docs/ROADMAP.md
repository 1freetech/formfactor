# FormFactor dependency-aware roadmap

Apply [simple instructions and component selection rules](INTERACTION_RULES.md) throughout this sequence. Preserve the lowercase `formfactor` name and the accuracy contract: unknown data stays unknown, approximations are labelled, and no electrical, thermal, manufacturing, or safety claim is made without an implemented validator or solver backed by authoritative inputs.

The SDL workbench is currently a visual smoke-test prototype. It proves that a native window can launch; it is not the target graphics quality and must not drive engineering truth. Physics and validated data stay below visualization in the dependency graph.

See [SEMICONDUCTOR_PHYSICS.md](SEMICONDUCTOR_PHYSICS.md) for the standing path from board/circuit physics through electromagnetic fields, semiconductor-device TCAD, quantum transport, and material/electronic-structure analysis.

## 1. Truth layer

Build the authoritative, reproducible engineering record before higher-level game behavior.

- Component provenance and accuracy tiers. (initial slice complete)
  - [Finite component-rating gate](COMPONENT_RATING_VALIDATION.md): voltage, current, and junction-temperature scalars reject NaN and infinity, with unit-labelled errors propagated to catalogue validation. Numeric validity is not manufacturer-rating verification or high-voltage/thermal certification.
- Versioned catalogue identity linking selected part, schematic symbol, footprint, physical model, simulation model, and explicit component-pin / footprint-pad / simulation-terminal mappings. (initial slice complete)
- Exact unit-aware quantities. ([sourced catalogue-property records and exact numeric filtering complete](CATALOG_QUANTITY_PROPERTIES.md); explicit schemas cover all four qualifiers for the nine currently supported non-dimensionless dimensions; conservative component-family applicability and exact property-to-part identity gates complete)
- Typed pins, nets, and topology-only electrical-rule validation. (initial slice complete)
- Deterministic three-state digital event simulator. (initial slice complete)
- SPICE/model integration. (canonical passive/DC operating-point export slice complete; solver execution and model-library support pending)
- Reproducible simulation records, solver/version provenance, deterministic inputs, and replayable outputs. (initial fail-closed SPICE execution-evidence record complete; solver invocation and semantic output validation pending)

Unit handling follows the BIPM SI Brochure and NIST SP 811. The current exact quantity slice stores decimal SI values without binary floating-point conversion and rejects unsupported exponent ranges rather than rounding them. Offset units such as degrees Celsius remain pending until affine conversion is implemented explicitly.

## 2. PCB and advanced-technology layer

Implement these in dependency order. A representation may exist before a solver, but it must never be reported as a solved physical result until a validated solver evaluates it.

- Schematic connectivity and stable cross-view component references.
- Board geometry: outlines, copper layers, pads, vias, traces, zones, keepouts, and mechanical objects.
- Multilayer stackups with sourced material properties. (initial representation complete)
- Exact geometry, sourced clearance, and per-trace current-limit checks. (partial implementation complete)
- Controlled-impedance requirements and differential-pair constraints. (requirement representation complete; field-solver evaluation pending)
- High-speed signal-integrity analysis only after validated transmission-line/field-solver interfaces exist.
- Return-path requirements. (representation complete; reference-plane geometry evaluation pending)
- Power integrity, target impedance, decoupling, and PDN analysis. (target-impedance and decoupling requirement representations complete; solver evaluation pending)
- Current density and calculated ampacity only after copper thickness, temperature rise, material properties, and thermal boundary conditions are sourced.
- Thermal vias, heat spreading, junction-to-board/package paths, and coupled thermal-power analysis only with validated thermal inputs.
- Creepage, clearance, high-voltage safety, pollution/material-group assumptions, and applicable standard provenance. Never infer a safe working voltage from spacing alone.
- EMI/EMC awareness and rule-based risk indicators first; field/radiated-emissions claims require validated solver or measured data.
- Flex and rigid-flex layer/stackup concepts, bend regions, material data, and bend constraints only after format and material support are explicit.
- BGA fanout, microvias, via-in-pad, filled/capped via attributes, and fabrication capability constraints.
- DFM/DFA/DFT gates, test-point coverage, fixture-access records, and boundary-scan/JTAG concepts. Boundary scan must use explicit device/BSDL or equivalent authoritative data; do not invent scan-chain capability.
- BOM, AVL, approved alternates, lifecycle/obsolescence risk, package/revision identity, and substitution safety.
- Repairability metrics based on explicit access, package, tooling, documentation, and replacement constraints.
- Manufacturing-yield models only when process capability distributions or measured production data are supplied; do not fabricate yield percentages.
- Measured-vs-modelled calibration records that preserve instrument, fixture, environmental, solver, model, revision, uncertainty, and timestamp provenance.
- KiCad-compatible import/export, DRC comparison, BOM/manufacturing bundles, and fabrication export gates.

## 3. Instruments and visualization

- Probes, multimeters, oscilloscopes, logic views, buses, and waveform history.
- Voltage/current/power/thermal overlays sourced from solver results, never renderer guesses.
- Pause, step, replay, run history, adjustable parameters, and exportable simulation records.
- Clearly labelled fast/approximate versus high-accuracy/non-realtime modes where validated solver backends support both.
- Separate CPU/GPU execution paths only when numerical equivalence/tolerance tests exist.

Interaction inspiration may come from CRUMB, EveryCircuit/Falstad, SpaceSim, Logic World, Virtual Circuit Board, Retro Gadgets, and similar tools, but proprietary code, assets, branding, and protected content are never copied.

## 4. Firmware and hardware co-simulation

- Deterministic firmware/peripheral co-simulation with observable pins, buses, clocks, interrupts, and reproducible timing boundaries.
- Explicit executable/firmware provenance and simulator versions.
- Verilator and other license-compatible open tooling may be integrated behind validated adapters where appropriate.
- No undocumented chip internals are invented.

Wokwi and Turing Complete are interaction/progression references only; formfactor must use original implementation or license-compatible open dependencies.

## 5. Responsive 2D then inspectable 3D workbench

- Build a responsive 2D engineering workbench first: labelled category buttons, component cards, placement/snapping, wiring/routing interactions, linked schematic/board selection, and readable validation results.
- Preserve a secondary `formfactor 2D Lite` runtime profile for systems with limited RAM, GPU capability, or storage. It should use the same project files and engineering truth layer while omitting optional 3D assets, heavy materials, and other presentation-only costs. (required future profile; not implemented yet)
- Prefer lazy/on-demand loading for component previews, textures, waveforms, and other nonessential assets so the full application does not require every visual resource in memory at once.
- Measure startup time, peak memory, GPU/CPU load, and project-open time before claiming the Lite profile is faster or lighter. Performance targets must come from repeatable benchmarks rather than estimates.
- Lite mode must not silently weaken ERC/DRC, electrical calculations, safety gates, or export rules. Any intentionally reduced-fidelity solver mode must be separately named, explicitly selected, and preserve its own provenance/limitations.
- Add high-fidelity 3D inspection only after authoritative package/footprint/physical-model links exist.
- Prefer manufacturer STEP models, verified KiCad-compatible 3D models, or models generated from sourced mechanical dimensions. Mark unverified assets `visual-only`.
- Use the best license-compatible open graphics stack available; Godot, FreeCAD, KiCad, and related open tools may be used when their interfaces and licenses fit the architecture.
- The renderer never determines whether a circuit works.

See [VISUAL_ASSET_PIPELINE.md](VISUAL_ASSET_PIPELINE.md).

## 6. Lessons, engineering contracts, metrics, and sandbox

- Progress from electrical fundamentals through component use, schematic capture, layout, verification, advanced PCB constraints, troubleshooting, and optimization.
- Include a semiconductor-physics progression from atomic/crystal concepts through bands, doping, PN junctions, MOS structures, device transport, and only then advanced quantum/material views. Connect lessons to [SEMICONDUCTOR_PHYSICS.md](SEMICONDUCTOR_PHYSICS.md) so educational explanations never outrun the implemented solver/data layer.
- Use realistic engineering briefs, datasheets, constraints, costs, availability, test evidence, and objective pass/fail outcomes.
- Preserve an unrestricted sandbox alongside guided progression.
- Add save/load/share only after file formats are versioned and reproducible.
- Optimization scores may include cost, area, power, thermal margin, manufacturability, repairability, part availability, and measured performance only when the inputs are explicit.

SHENZHEN I/O, Turing Complete, CRUMB, and PC Building Simulator are design references for learning flow and tactile interaction only.

## 7. Advanced solvers and calibration

Add advanced thermal, power, signal-integrity, EMI/EMC, electromagnetic/field, semiconductor-device, quantum-transport, and material/electronic-structure analysis only when each solver has:

1. documented equations or an authoritative upstream interface;
2. explicit units and boundary conditions;
3. solver/version provenance;
4. reference fixtures or analytical cases;
5. numerical tolerance tests;
6. uncertainty/limitation reporting;
7. deterministic or reproducibly bounded execution;
8. measured-vs-modelled calibration support where real measurements exist.

The intended solver ladder is documented in [SEMICONDUCTOR_PHYSICS.md](SEMICONDUCTOR_PHYSICS.md): SPICE/ngspice for circuit/compact models; a validated field solver such as openEMS for PCB electromagnetics; a TCAD interface such as DEVSIM for explicitly defined semiconductor devices; quantum-transport tools such as Kwant only for supplied microscopic models; and electronic-structure tools such as Quantum ESPRESSO only for explicit material/crystal calculations. These are candidate open interfaces, not claims of current integration.

ngspice/SPICE, Qucs-S, Verilator, OpenROAD, KiCad, FreeCAD, and similar open tools may be integrated as license-compatible upstream engines or workflow references. Solver output must remain separable from visualization.

## 8. FormFactor training module and virtual certification

This is a required future product pillar, not a current feature.

- Training modules must teach the same validated engineering concepts used by the simulator.
- Completion requires objective checks rather than cosmetic progress alone.
- A virtual formfactor certificate may be generated only after the defined completion requirements pass.
- Email delivery of a certificate requires an explicit user-provided destination, user consent, and a configured/authorized mail-delivery interface. The project must not claim an email was sent unless the delivery service confirms it.
- Certificate records should include curriculum version, completion timestamp, assessment version, and verification identifier without exposing unnecessary personal data.

## Standing development protocol

For each bounded development run:

1. Read current `main`, this roadmap, `INTERACTION_RULES.md`, `GETTING_STARTED.md`, tests, open gaps, and recent commits.
2. Select exactly one improvement from the earliest incomplete dependency. State the pillar and the advanced-PCB capability it unlocks.
3. Define measurable acceptance criteria and the authoritative standard, equation, file format, or validated upstream interface used.
4. Write original code or use only license-compatible open dependencies.
5. Test valid, invalid, boundary, deterministic/replay, unit, and export-safety behavior as applicable.
6. Run the complete available validation suite on actual repository files; use strict warnings and sanitizers where supported.
7. Fix failures before moving the tested change to `main`.
8. Update status only for behavior actually implemented and tested. Keep future requirements labelled pending.
9. Verify the resulting GitHub `main` state after the passing commit lands.
10. Report only concrete tested changes, acceptance results, limitations/blockers, commit hash, and verification link in simple but scientifically precise language.

If authoritative data, permissions, credentials, dependencies, licensing, or safe architecture are missing, record the exact blocker and choose a smaller independently verifiable improvement in the same dependency layer.
