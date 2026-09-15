# FormFactor Inventory and Component Library Design

Updated: September 15, 2026

Status: production inventory design contract. This document defines how FormFactor should grow from the current six-part prototype palette into a large engineering component library without turning the temporary SDL2 FreeLab into a dead-end UI.

## Research references

Two games are especially useful references for this system.

### Stationeers: depth and technical grouping

Stationeers is useful because its electronics are not treated as one flat list. Its component ecosystem is split into meaningful technical families such as circuitboards, integrated circuits, logic I/O, logic memory, processors, switches, transmitters, motherboards, sensors, cables, consoles, computers, and related electronic parts.

FormFactor should borrow the general idea of a deep technical catalogue with meaningful families and device details. It should not copy Stationeers art, UI, code, names, assets, or proprietary implementation.

### Satisfactory: finding things quickly

Satisfactory is useful because a very large build library remains manageable through categories, subcategories, quick search, multiple hotbars, an in-game Codex, sorting, contextual shortcuts, and copy/eyedropper behavior.

FormFactor should borrow those general interaction ideas: search first, fast reuse, quick access, useful categories, and one-click access to deeper information. It should not copy Satisfactory art, UI, code, branding, or proprietary implementation.

## Current problem

The temporary FreeLab workbench currently exposes six hard-coded prototype choices:

- resistor;
- capacitor;
- LED;
- integrated circuit;
- power source;
- connector.

That was enough to prove the first graphical loop, but it will not scale to a real PCB-building game.

The production inventory must be driven by the authoritative component catalogue instead of a fixed `PartKind` list in the renderer.

## Inventory structure

The left-side component library should support these top-level groups.

### Favorites

Parts the player pins for repeated use in the current project or across projects.

### Recent

Recently placed or inspected parts. This keeps common work fast without forcing the player to search again.

### Passive components

Examples include:

- resistor;
- potentiometer / variable resistor;
- ceramic capacitor;
- electrolytic capacitor;
- film capacitor;
- inductor;
- ferrite bead;
- transformer when supported by validated data and models.

### Diodes and semiconductors

Examples include:

- signal diode;
- rectifier diode;
- Zener diode;
- LED;
- bridge rectifier;
- NPN transistor;
- PNP transistor;
- N-channel MOSFET;
- P-channel MOSFET;
- optocoupler.

### Integrated circuits and logic

Examples include:

- logic gate;
- buffer / inverter;
- timer;
- comparator;
- operational amplifier;
- voltage reference;
- driver;
- memory device;
- microcontroller;
- programmable logic device.

A generic label must never imply that every package, pinout, voltage, timing value, or model is interchangeable. Exact selectable parts must come from catalogue records.

### Power and protection

Examples include:

- battery / DC source;
- ground / reference symbol;
- fuse;
- resettable fuse;
- switch;
- relay;
- linear regulator;
- buck regulator;
- boost regulator;
- load switch;
- TVS protection device.

### Connectors and interfaces

Examples include:

- pin header;
- terminal block;
- barrel jack;
- board-to-board connector;
- USB connector;
- test point;
- programming header.

Connector entries must preserve exact pin mapping and package identity.

### Sensors and inputs

Examples include:

- push button;
- toggle switch;
- rotary encoder;
- thermistor;
- photoresistor;
- temperature sensor;
- Hall-effect sensor;
- current sensor;
- voltage-sense element.

### Electromechanical and outputs

Examples include:

- relay;
- buzzer;
- speaker;
- motor;
- fan;
- solenoid;
- display;
- indicator lamp.

### Tools and instruments

Tools are separate from parts placed into the electrical design. Examples include:

- multimeter;
- oscilloscope;
- logic probe;
- continuity tester;
- bench power supply;
- waveform / signal source.

This category changes the active tool, not the PCB netlist.

## Component card

Every selectable part card should show enough information to make the choice understandable before placement.

Minimum card fields:

- schematic symbol;
- physical/package thumbnail when available;
- component name;
- reference prefix such as `R`, `C`, `D`, `Q`, `U`, `J`, or `F`;
- exact package or footprint when known;
- one or two key electrical limits or values when sourced;
- model availability;
- trust state: `verified`, `partial`, or `visual-only`.

Unknown information must be shown as unknown, not guessed.

## Search and filters

Search must be the fastest way to reach a large library.

Search should match:

- common name;
- manufacturer part number;
- reference prefix;
- family;
- package;
- footprint;
- electrical function;
- tags.

Useful filters include:

- component family;
- through-hole / SMD;
- package / footprint;
- voltage range;
- current range;
- resistance / capacitance / inductance range;
- verified / partial / visual-only;
- has simulation model;
- has physical 3D model;
- favorites;
- recently used.

Only filters backed by actual catalogue properties should be displayed.

## Quick access

Borrow the general idea of build hotbars without copying another game's UI.

FormFactor should support:

- 10 quick slots per bank;
- several switchable banks;
- drag or explicit assign-to-slot behavior;
- keyboard and gamepad access;
- project-specific quick slots;
- optional global favorites.

The player should also be able to point at an already placed part and use a **Pick Same Part** action that selects the exact same catalogue identity, package, and supported configuration where safe to copy.

## Details / engineering Codex

Every inventory card should open a Details view that can show, when available:

- symbol;
- footprint;
- 3D model;
- package dimensions;
- pinout;
- electrical limits;
- simulation-model status;
- manufacturer/source information;
- provenance;
- accuracy tier;
- compatible training exercises;
- known missing data.

This is the FormFactor equivalent of an engineering encyclopedia. It is not a separate truth source; it reads from the same catalogue records used by validation.

## Inventory interaction rules

1. **Search before scrolling.** A player should not need to scan hundreds of cards manually.
2. **Categories stay technical and predictable.** Do not hide electrical parts behind arbitrary game-only categories.
3. **Symbols and physical parts stay linked.** Selecting a component selects one stable identity across schematic, PCB, 3D, instruments, and validation.
4. **Exact parts beat vague icons.** A generic family can be used for browsing, but placement must resolve to an exact supported catalogue item when engineering truth depends on exact properties.
5. **No fake availability.** A part with missing required model/data may be browsable but must clearly state what it cannot yet validate.
6. **Favorites do not duplicate records.** They point to the same stable catalogue identity.
7. **Recent history is reversible UI state.** It does not change engineering data.
8. **Large libraries load progressively.** Heavy thumbnails and 3D assets may load on demand while catalogue identity and validation metadata remain immediately available.
9. **Keyboard and gamepad parity.** Search, categories, filters, cards, quick slots, and Details must be reachable without a mouse.
10. **No color-only status.** Verification state needs text/icon plus color.

## Production architecture requirement

The current SDL2 prototype uses a fixed `PartKind` enum and six-item array. That implementation is a smoke-test interface only.

The production SDL3 + bgfx + Dear ImGui frontend should request component records from the catalogue through a stable inventory query API. The renderer should not own the list of available engineering parts.

Conceptually:

`catalogue records -> inventory query/filter -> component card -> placement request -> core validation`

The placement request carries the stable component identity and selected package/variant. The UI never fabricates missing properties.

## First inventory expansion target

Before trying to expose hundreds of manufacturer-specific parts, FormFactor should make the inventory system capable of handling at least these common families cleanly:

1. resistor;
2. potentiometer;
3. ceramic capacitor;
4. electrolytic capacitor;
5. inductor;
6. standard diode;
7. Zener diode;
8. LED;
9. NPN transistor;
10. PNP transistor;
11. N-channel MOSFET;
12. P-channel MOSFET;
13. logic gate / inverter;
14. op-amp;
15. comparator;
16. voltage regulator;
17. fuse;
18. switch;
19. relay;
20. connector / header;
21. test point;
22. sensor;
23. buzzer / small output device;
24. power source;
25. ground / reference symbol.

This list is an inventory and data-model target, not a claim that all 25 are already simulated or validated.

## Acceptance tests

- The inventory can display more than six component types without changing renderer source for each new catalogue item.
- Search can find an exact component by name or part number.
- Categories can be navigated by mouse, keyboard, and gamepad.
- Favorites and Recent point to stable catalogue identities.
- A component card shows its trust state and model availability.
- Selecting Details exposes provenance and missing-data status.
- Pick Same Part selects the same catalogue identity instead of a visually similar substitute.
- Filtering cannot silently hide an unknown property by pretending it failed a numeric comparison.
- A visual-only model cannot become electrically verified because it appears in the inventory.
- Adding a new catalogue record does not require adding a new hard-coded renderer enum value.

## Source notes

Research references used for the interaction direction include the Stationeers electronics/device taxonomy and the Satisfactory build menu, quick search, hotbars, Codex, inventory sorting, categories, and blueprint organization. These are high-level design references only. FormFactor does not copy proprietary code, UI artwork, assets, names, or implementation details from either game.
