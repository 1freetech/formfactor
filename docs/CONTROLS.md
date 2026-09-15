# FormFactor Controls

This page explains how to move around the current FormFactor prototype.

## Quick start

1. Pick a part from the left side.
2. Click an empty place on the green board to place it.
3. Look beside the physical part. Its schematic symbol appears next to it.
4. The same symbol also appears in the **Live Schematic Mirror** on the right.
5. Right-click one placed part, then right-click another part to connect them.
6. Press **V** or click **TEST** to check the current prototype rule.
7. Press **C** or click **CLEAR** to start over.

## Mouse controls

- **Left-click a part:** choose that part.
- **Left-click empty board space:** place the chosen part.
- **Left-click a placed part:** focus that part and read about it.
- **Right-click one placed part, then another:** connect the two parts.
- **Click a CONNECT option on the right:** connect the focused part to that listed part.
- **Click TEST:** test the current board.
- **Click CLEAR:** remove everything from the board.
- **Click HELP:** open or close the in-game instruction page.

## Keyboard controls

- **Tab:** move to the next clickable group.
- **Shift + Tab:** move back one group.
- **Arrow keys:** move through parts or placed board items.
- **Enter or Space:** use the focused item.
- **V:** test the board.
- **C:** clear the board.
- **H or F1:** open or close the help page.
- **Esc:** exit FormFactor.

## What each area does

### Left side: Physical parts and symbols

Each part shows two things together:

- what the physical part looks like
- what its schematic symbol looks like

Current prototype parts are:

- **R — Resistor:** axial through-hole resistor
- **C — Capacitor:** radial electrolytic capacitor
- **LED / D — LED:** 5 mm through-hole LED
- **IC / U — Chip:** DIP integrated circuit
- **PWR / BT — Power:** battery-style power source
- **J — Connector:** 1 x 6 pin header

## Center: Physical board

The center is the board-building area.

When you place a part:

- the physical part appears on the board
- a reference name appears near it, such as `R1`, `C1`, or `U1`
- its schematic symbol appears in a small badge next to the physical part
- the same part appears in the live schematic on the right

This lets you learn the physical part and schematic symbol at the same time.

## Right side: Live schematic and part codex

The right side has three jobs.

### Live Schematic Mirror

It shows the same parts you placed on the physical board using schematic symbols.

When you connect two physical parts, the schematic mirror adds a line between their symbols.

The current schematic layout follows the rough board position to make learning easier. A later version can use a smarter schematic layout.

### Part Codex

The codex gives a one-sentence explanation of the selected part and shows its current prototype package type.

### Available to Connect

When you focus a placed part, FormFactor lists placed parts that the current training rules allow you to connect to.

These are **prototype training rules**, not final pin-level electrical rules.

Final electrical compatibility must come from verified component data, pinouts, ratings, and engineering checks.

## Basic prototype test

The first simple test currently checks for a powered load.

A basic passing example is:

**Power → Resistor**

or

**Power → LED**

The simulator will become more strict as real component pin rules and circuit checks are connected to the visual workbench.

## Symbols and physical parts

FormFactor is being built so a player does not have to memorize symbols before using the simulator.

The goal is:

**physical part + schematic symbol + short explanation + connection guidance**

all visible together while you build.
