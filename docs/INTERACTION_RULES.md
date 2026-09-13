# Simple instructions and accurate component selection

Status: project requirements. The current application is a terminal engineering core. The component catalogue, selection cards, linked schematic/board views, and guided game interface below are planned work, not completed features.

## 1. Make each step easy to follow

1. Prefer visible buttons and menu names. For example: click **Extensions**, search **WSL**, then click **Install**.
2. Number the steps and put one action in each step. Shortcuts and extension IDs belong in optional details.
3. State prerequisites before the first action. Explain where each command is entered and what it does.
4. Show the expected result after each short group of steps. Use the user's actual screen or error to select the next action.
5. Explain an unfamiliar term briefly when it first matters. Keep correct names and units visible.
6. Offer engineering detail through **Details** or **Why?** without hiding errors, limits, missing data, or simulation status.
7. If a step fails, identify the problem and give one concrete next action. Preserve the user's work and report only results that were actually saved or verified.

These rules apply to setup guides, build instructions, development reports, tutorials, and the game itself. Use an adult, respectful tone with clear language. Simplicity must preserve the engineering accuracy contract.

## 2. Group parts by electrical function

Use electrical function as the main category. Search should accept familiar component names, synonyms, and exact part numbers. The following is the planned starting taxonomy; it does not imply that these parts are currently modelled.

| Category | Component families | Useful filters within the family |
| --- | --- | --- |
| Resistors, capacitors and magnetics | Resistors, capacitors, inductors, transformers | Resistance, capacitance, inductance, tolerance, ratings |
| Diodes and protection | Rectifiers, Zener diodes, TVS devices, fuses | Reverse voltage, forward current, clamping or trip characteristics |
| Switching and amplification | Transistors, MOSFETs, amplifiers, comparators | Device type, channel, gain, on-resistance, bandwidth |
| Power supply and conversion | Batteries, regulators, power modules | Input/output voltage, current capability, regulation type |
| Logic, computing and memory | Logic gates, controllers, processors, memories, ASICs | Logic family, supply range, interfaces, clock limits |
| Sensors and timing | Temperature/current/light sensors, crystals, oscillators | Measured quantity, range, interface, frequency |
| Connections and controls | Connectors, switches, relays, test points | Pin count, pitch, contact ratings, mounting style |
| Indicators and actuators | LEDs, displays, buzzers, motors | Drive requirements, output type, dimensions |

A part has a primary family and may have multiple function tags. For example, an LED remains one part record even when found through both "diode" and "indicator" searches. A module is identified as a module, with its supported level of simulation stated.

Apply physical filters across families: package, dimensions, pin count, pitch, mounting style, polarity where relevant, and operating temperature. Expose only electrical filters meaningful for the selected family. Compare units correctly and preserve operating conditions and tolerances; for example, a rating at one temperature is not automatically a rating at every temperature. Missing values stay unknown and cannot satisfy a requested numeric constraint. Category membership is not evidence of compatibility or physical performance.

## 3. Show one selected part consistently

Each catalogue selection must clearly show:

| Visible field | Meaning |
| --- | --- |
| Name and purpose | Familiar component name and one short explanation |
| Symbol | Correct schematic representation for the selected part and variant |
| Physical preview | Selected package and board pad pattern, called its footprint |
| Identity | Manufacturer, full part number, package variant and catalogue revision |
| Value and limits | Relevant values with units, tolerance and conditions |
| Pins | Pin numbers, functions, polarity and orientation markers |
| Model status | Verified, partial or visual-only, with the available simulation scope and missing data explained |
| Evidence | Sources and model revisions, available through Details |

A generic symbol, an ideal teaching model, and a sourced physical component must be visibly distinguishable. A resistor symbol alone does not identify a particular manufacturer, resistance, power rating, or package. One part may support several symbol styles or packages; each supported assignment must be explicit.

## 4. Keep the symbol, package and model connected

1. Give each placed component one stable instance identity and a readable reference such as `R1`, `C1`, or `U1`.
2. Link that instance to versioned catalogue, symbol, footprint, physical-model and simulation-model records. Missing records must be marked unavailable.
3. Match schematic pin numbers to the selected package's footprint pad numbers and manufacturer pinout. Pin count and appearance alone do not establish compatibility.
4. Map simulation-model terminals explicitly; a solver's terminal order may differ from the package numbering.
5. Highlight the same component and connection when selected in the catalogue, schematic, board, or instrument view. Respect rotation, polarity, and multi-unit symbols belonging to one physical package.
6. When the part, package or value changes, update all linked views together and invalidate affected simulation and validation results. Never present results from the old selection as current.
7. Treat ground labels, net labels and other schematic-only objects as connection tools, not physical inventory items. Keep mounting holes and other mechanical objects identifiable by their actual role.

These mapping requirements follow the distinction between symbols, footprints and model terminals documented in the [KiCad schematic editor manual](https://docs.kicad.org/9.0/en/eeschema/eeschema.html). Actual catalogue records still require authoritative data for the specific part and package.

Physical previews with missing dimensions or assets must show their limitations. An approximate illustration must not count as evidence that mechanical clearances pass. Importing a part does not automatically provide accurate simulation of its internal circuitry. An unverified mapping prevents a fabrication-ready result under the existing accuracy contract.

## 5. Make the game follow the same numbered pattern

Example of the intended component-selection flow, after the catalogue exists:

1. Click **Components**. The category list opens.
2. Click a category. Its available parts appear.
3. Select a part. Its symbol, package, values and model status appear together.
4. Click **Place**. The chosen part follows the pointer.
5. Click a location. The part is placed with its reference label.
6. Click **Check**. Implemented checks report their results and anything still unknown.

Use a visible current step, clear labels, and a next-action button in guided mode. Provide text alongside icons and status colours. Keep unrestricted sandbox access available. Simulation and fabrication actions must state what is actually supported; a successful placement check cannot stand in for electrical, thermal or manufacturing validation.

## 6. Build and verify this in order

1. Add catalogue identities, functional categories and property records to the engineering data model.
2. Add sourced symbol-to-package and symbol-to-model mappings.
3. Add search and property filtering, with documented handling of missing values.
4. Add a 2D selection card showing the linked records.
5. Add shared selection and numbered guidance to the workbench.
6. Add verified physical previews and 3D inspection when their dependencies exist.

Acceptance cases for those future implementations:

- A name, synonym or part-number search resolves the same catalogue identity; categories do not duplicate physical parts.
- Equivalent units produce equivalent filtering. Missing data, temperature conditions and tolerance are not silently discarded.
- A familiar-looking package with an incompatible pinout is rejected as a valid assignment.
- An intentionally different solver terminal order works only through its explicit mapping; a missing or incorrect mapping is rejected.
- Selection, reference labels and changed values agree across schematic and board views. Changing a package invalidates old results.
- Ground labels create connections without fictitious packages or purchased parts.
- Guided instructions use the visible control names and identify the expected result. Detailed help retains the engineering facts.
- Unavailable features and unverified data stay visibly unavailable or unknown, and cannot satisfy a fabrication gate.

Each hourly development run must read these rules alongside the roadmap, choose one bounded improvement in dependency order, validate the applicable behavior locally, and verify the saved GitHub result. Report completed behavior separately from requirements and future work.
