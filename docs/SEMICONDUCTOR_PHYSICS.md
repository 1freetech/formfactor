# FormFactor semiconductor and microscopic physics ladder

Status: standing architecture requirement. This document defines how formfactor may eventually descend from board-level electrical behavior into semiconductor device physics, quantum transport, and materials physics without mixing scales or inventing microscopic behavior.

## 1. Why this exists

formfactor is intended to remain useful from technician training through advanced engineering study. The long-term goal is not only to show that a transistor, diode, regulator, or ASIC package exists on a PCB, but to let a user inspect the physics layer that is appropriate to the question being asked.

A PCB trace, return path, via, differential pair, decoupling network, and power plane are primarily classical circuit/electromagnetic problems. A PN junction or MOS structure is a semiconductor-device problem. Nanoscale transport and electronic structure require still deeper models. formfactor must therefore use a hierarchy of validated solvers rather than a single universal "physics engine."

## 2. Physics ladder

### Level A — circuit and package behavior

Primary tools/interfaces: formfactor deterministic models, SPICE/ngspice, authoritative compact models and measured data.

Use for:
- voltage, current, power, resistance, capacitance, inductance and timing;
- transistor/diode/regulator behavior through sourced compact models;
- package parasitics when authoritative values or extracted models exist;
- board-level co-simulation and ordinary gameplay.

This is the normal runtime layer. Deeper physics is not required for every component evaluation.

### Level B — PCB electromagnetic fields

Candidate open solver: openEMS or another validated, license-compatible field solver.

Use for:
- controlled impedance and transmission lines;
- differential pairs and coupling;
- return-current paths and discontinuities;
- via transitions and field distribution;
- antennas, shielding, resonances and EMI/EMC research;
- extracting field-derived parameters that can feed higher-level circuit models.

A renderer must never fabricate electric or magnetic field patterns. Visual overlays must come from a solver result with units, mesh/boundary conditions, solver version, and known limitations.

### Level C — semiconductor device physics

Preferred candidate: DEVSIM or another validated open TCAD interface.

Use for explicitly defined devices such as PN junctions, MOS structures, diodes, BJTs, MOSFET teaching structures, and solar cells where the geometry, materials, contacts, doping, mobility/recombination assumptions, boundary conditions, and equations are known.

Potential observable fields include:
- electrostatic potential;
- electric field;
- electron and hole concentrations;
- current density;
- depletion/inversion/accumulation regions;
- carrier transport;
- transient or AC device response;
- quantum-correction terms only when the underlying model is explicitly selected and documented.

formfactor must not reverse-engineer or invent the internal geometry, doping profile, material stack, transistor count, or process details of a commercial IC/ASIC when those data are not public. In that case the package remains represented by verified external data, compact models, measurements, or explicitly labelled behavioral models.

### Level D — quantum transport

Candidate open solver: Kwant or another validated quantum-transport package.

Use only for intentionally defined nanoscale structures where a tight-binding Hamiltonian or equivalent microscopic model is actually supplied. Possible outputs include transmission, conductance, modes, wavefunctions, scattering matrices, Green functions, and related quantities.

Quantum transport is an advanced inspection/research mode, not the default explanation for ordinary PCB current flow.

### Level E — material and electronic structure

Candidate open suite: Quantum ESPRESSO or another validated DFT/electronic-structure engine.

Use only where crystal structure, pseudopotentials, basis/cutoff choices, exchange-correlation treatment, convergence criteria, and material assumptions are explicit. Possible use cases include band structure, density of states, phonons, electron-phonon calculations, and research-grade material properties.

Results at this level are computational research outputs. They must never be presented as exact properties of a manufactured component unless the material/process identity and validation evidence justify that claim.

## 3. Solver handoff rule

Deeper solvers should feed validated reduced-order information upward instead of running everywhere all the time.

Examples:
- a field solver may extract impedance or coupling parameters that become a circuit model;
- a TCAD run may generate or calibrate an I-V/C-V relationship used in a teaching device model;
- quantum/material calculations may inform a research fixture, but ordinary board simulation continues to use compact or measured models.

This keeps formfactor responsive while preserving a path to high-accuracy analysis.

## 4. Inspect Physics interaction model

A future component or structure may expose an **Inspect Physics** action with progressively deeper views when evidence exists:

1. **Board/Circuit** — pins, nets, voltage, current, power, compact model and model provenance.
2. **Fields** — E/H/current-density/thermal overlays generated by an appropriate solver.
3. **Device** — junction/channel/contact/carrier behavior for a defined semiconductor structure.
4. **Quantum** — transport/wavefunction-level analysis for a defined nanoscale model.
5. **Material** — electronic-structure calculations for an explicitly defined material system.

Unavailable layers stay unavailable. formfactor must never fill a missing layer with guessed internals.

## 5. Data and reference hierarchy

Engineering truth should prefer, in order appropriate to the question:

1. manufacturer datasheets, package drawings, IBIS/SPICE/BSIM/compact models and process documentation;
2. standards organizations, NIST/BIPM and other authoritative metrology/safety references;
3. solver manuals and upstream validation suites;
4. peer-reviewed papers, textbooks and university course material;
5. measured fixture data with instrument/calibration provenance;
6. BitcoinVersus.Tech technical articles as an educational/reference index and training source;
7. Wikipedia and similar encyclopedic resources for orientation, terminology, and source discovery only.

Wikipedia or a general educational article must not be the sole numerical authority for a fabrication, safety, model-validation, or certification gate.

## 6. BitcoinVersus.Tech training linkage

BitcoinVersus.Tech already contains a substantial semiconductor/solid-state knowledge base, including topics such as PN junction current, forward bias, diode ideality factor and series resistance, MOSFET threshold voltage and operating characteristics, oxide charge density, flat-band capacitance, polycrystalline-silicon resistance, lattice atoms, valence bands, solar cells, characterization methods, and semiconductor processing concepts.

Future formfactor training modules may link those articles to the corresponding simulator objects and solver views. Educational prose may explain the concept; the simulator remains responsible for showing which variables, equations, boundary conditions, source data, and solver actually generated a result.

## 7. Performance profiles

The physics ladder must support progressive computation:

- `2D Lite`: board/circuit truth plus lightweight visualization; optional deeper solvers run only on demand.
- `Standard`: normal circuit simulation, responsive 2D/3D inspection, and selected extracted models.
- `Engineering`: higher-resolution field/device runs with explicit solver provenance.
- `Research`: non-realtime TCAD/quantum/material workflows where supported.

These names are provisional product requirements, not implemented runtime modes. Accuracy claims depend on the selected solver/model and validation evidence, not the label of the profile.

## 8. Dependency order

Do not implement this stack backwards.

1. Finish exact quantities, catalogue properties, provenance, netlists/ERC and reproducible simulation records.
2. Stabilize SPICE/model integration and board geometry.
3. Add validated field-solver interfaces for PCB electromagnetics.
4. Add a TCAD/device-physics adapter with small analytical/reference fixtures.
5. Add quantum transport only after the device-layer abstraction and provenance/replay format are mature.
6. Add DFT/material workflows only as an advanced research module.
7. Connect each layer to the training system and visual inspection only after the solver/data path is validated.

Every level must preserve inputs, solver/version, boundary conditions, tolerances, convergence status, outputs, warnings, and reproducible records.
