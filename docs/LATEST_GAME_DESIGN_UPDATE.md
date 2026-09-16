# Latest FormFactor Game Design Update

Updated: September 15, 2026

This page is the short version of the latest FormFactor game-design direction. It combines the useful lessons from the game research without copying proprietary code, art, audio, characters, missions, branding, menus, broadcast packages, maps, or protected level design.

## Current concentration: gameplay feel

The immediate game-facing focus is now **making FormFactor realistic, easy to control, responsive, and genuinely fun to play**.

The engineering core still decides truth. The frontend should make that truth easy and satisfying to interact with.

The current priority is:

1. immediate visible response to input;
2. simple direct controls for select, place, move, rotate, connect, cancel, delete, test, and inspect;
3. placement ghosts, snapping, target highlights, and connection previews before committing;
4. safe cancel plus undo/redo for normal editing mistakes;
5. very fast retry after failed tests or bad edits;
6. clear feedback showing exactly what changed after every important action;
7. realistic results without forcing unnecessary simulator complexity into every basic action;
8. short satisfying confirmation for placement, connection, measurement, repair, and validation;
9. multiple valid engineering solutions instead of one scripted answer;
10. high-quality graphics and 3D only after the basic 2D engineering loop already feels good.

See [Gameplay Feel Requirements](GAMEPLAY_FEEL_REQUIREMENTS.md) for the fresh all-time gameplay scan and the concrete production requirements.

## What FormFactor should feel like

FormFactor should be easy to start, satisfying to use, and deep enough to grow with the player.

The player should be able to:

- open FreeLab and experiment immediately;
- start a guided Engineering Contract when they want structure;
- practice one engineering skill without replaying a full mission;
- fail, understand why, and retry quickly;
- move between schematic, PCB, instruments, validation, and later 3D without losing context;
- build from working templates instead of always starting from an empty screen;
- search a large component library instead of scrolling through a tiny fixed palette;
- favorite, filter, and quickly reuse common parts;
- see the known strengths, limits, and missing evidence of a design before final validation;
- save useful subsystems as reusable Engineering Blueprints;
- navigate very large projects without loading every visual detail at once;
- eventually create and share safe custom engineering challenges;
- see clear feedback after important actions;
- improve through real engineering evidence instead of cosmetic points.

## What each game family adds

### Fresh all-time gameplay scan

A fresh comparison of current expert and aggregate greatest-game lists reinforces the same gameplay fundamentals across very different genres. The most useful reference set for FormFactor includes Tetris, Super Mario 64, Super Mario World, The Legend of Zelda: Ocarina of Time, Doom, Resident Evil 4, Half-Life 2, Portal 2, Minecraft, and The Legend of Zelda: Breath of the Wild.

The common lessons are:

- controls must feel trustworthy;
- the first useful action should happen quickly;
- inputs should have obvious cause and effect;
- the core loop should be enjoyable before progression systems are added;
- failure should be understandable and cheap to retry;
- systems should combine into deeper play without hiding basic rules;
- player freedom and multiple valid solutions create replay value;
- presentation should make actions feel good without slowing them down.

### Highly rated games in general

The broader game research adds the base rules:

- responsive controls;
- a strong repeatable loop;
- clear goals;
- player choice;
- useful failure feedback;
- strong visual polish;
- accessibility from the beginning;
- presentation that makes actions feel important without getting in the way.

### Grand Theft Auto series

The GTA research adds the open-world structure:

- FreeLab stays open even when guided content exists;
- Engineering Contracts use the same world/project state as the sandbox;
- contracts are broken into stages with meaningful checkpoints;
- failed work can restart from the nearest valid checkpoint;
- one project can be viewed from several perspectives without losing selection or objective state;
- large presentation assets can stream in only when needed.

### Fortnite Creative / UEFN

The Fortnite research adds creator workflow ideas:

- start from working templates;
- use reusable objectives, checkpoints, timers, faults, budgets, and validation modules;
- support a fast edit -> playtest -> edit loop;
- keep creator tools simple first, with deeper scripting later;
- let authors test exactly what a learner will experience.

### Roblox

The Roblox research adds platform and creation ideas:

- building, testing, and scripting should feel connected;
- projects should work across different window sizes and input styles;
- optional presentation content should stream without changing engineering state;
- community content needs version, origin, license, dependency, and trust information;
- future scripting must be sandboxed and least-privilege.

### MLB The Show

The MLB The Show research adds training and performance feedback:

- an Engineering Practice Lab for one skill at a time;
- Learn, Standard, Advanced, and Expert assistance profiles;
- help changes hints and labels, never engineering truth;
- short action-feedback cards after important measurements, placements, or repairs;
- targeted drills when recorded mistakes repeat;
- process-aware scoring for correct tool choice, safe procedure, diagnosis, and validation;
- long-term mastery tracking based on real completed work;
- polished camera, overlay, audio, and replay presentation for important engineering moments.

### NBA 2K

NBA 2K adds clearer build planning and visible tradeoffs:

- a **Design Scouting Report** before final validation;
- show known strengths, limits, missing data, and blocked requirements;
- let a player choose goals such as low cost, small area, lower power, or repairability;
- show requirement thresholds only when FormFactor has authoritative data for them;
- make tradeoffs between cost, size, thermal margin, repairability, and performance easy to understand;
- use project milestones to celebrate real recorded engineering progress;
- support clearly labelled fast/standard/full analysis profiles where validated backends allow them.

The scouting report explains what is known. It never predicts a pass that the engineering core has not proven.

### Spore

Spore adds modular creation and project evolution:

- direct manipulation should make component placement and editing feel simple;
- parts must change behavior through real component data, not cosmetic RPG stats;
- one project should grow naturally from component to circuit to PCB to tested system;
- keep a **Project Evolution Timeline** of important changes, measurements, failures, repairs, and validation results;
- procedural visuals may be used later, but generated approximations remain partial or visual-only unless authoritative data supports them.

### Space Engineers

Space Engineers adds several especially relevant engineering-game ideas:

- FreeLab and constrained contracts should use the same physics/validation rules;
- save reusable subsystems as **Engineering Blueprints**;
- treat damage, diagnosis, repair, and re-validation as one continuous system;
- use inspection markers and measurement bookmarks across linked views;
- support community scenarios and blueprints with strong provenance, license, dependency, and trust metadata;
- allow future automation/scripting only through safe FormFactor APIs;
- keep creator/community content separate from authoritative engineering truth.

### Stationeers + Satisfactory

These two games add the strongest inventory-library ideas for FormFactor.

**Stationeers** contributes technical depth: electronics are organized into meaningful families instead of one flat list. FormFactor should likewise separate passive components, semiconductors, logic/ICs, power/protection, connectors, sensors, electromechanical parts, and tools.

**Satisfactory** contributes findability: large build libraries remain usable through categories, subcategories, quick search, hotbars, a Codex, sorting, contextual shortcuts, and copy/eyedropper behavior.

FormFactor translation:

- replace the production six-item palette with a catalogue-driven component library;
- add Favorites and Recent groups;
- add search by name, manufacturer part number, package, family, function, and tags;
- add filters for package, through-hole/SMD, electrical ranges, model availability, and trust state when catalogue data exists;
- show symbol + physical/package preview together;
- show `verified`, `partial`, or `visual-only` on every part card;
- add several quick-slot banks for commonly used parts;
- add **Pick Same Part** to select the exact same catalogue identity from an already placed component;
- add a Details / engineering-Codex view with pinout, footprint, limits, provenance, models, and missing-data status;
- make adding a catalogue record independent from adding a new hard-coded renderer enum.

### Extreme-scale game worlds

There is no honest single ranking for the biggest game map because games measure scale differently. For FormFactor, three useful extreme-scale references are:

- **Elite Dangerous**: hierarchical indexing, search, filters, region grouping, route planning, and navigation across a huge data space;
- **No Man's Sky**: procedural generation, LOD, asset streaming, memory control, and seamless movement between scales;
- **Minecraft**: chunk-based loading and a clean separation between render distance and simulation distance.

FormFactor translation:

- large projects are divided into logical project regions;
- heavy 3D assets can load only when needed;
- far-away visual detail can use lower LOD;
- analysis scope is separate from presentation loading;
- global engineering state remains authoritative even when a region is not visible;
- navigation should support whole-project -> subsystem -> component zoom levels.

## Combined FormFactor game systems

The current design direction now includes these major game-facing systems:

1. **Responsive direct controls** - fast select, place, move, rotate, connect, cancel, delete, test, and inspect actions.
2. **Placement/connection previews** - ghost placement, snapping, target highlights, and live connection previews before committing.
3. **Undo/redo and safe cancel** - normal mistakes are cheap to correct.
4. **FreeLab sandbox** - unrestricted experimentation using the real engineering core.
5. **Engineering Contracts** - guided multi-stage jobs using the same project state as FreeLab.
6. **Engineering Practice Lab** - short focused drills for one skill at a time.
7. **Scalable Component Library** - searchable catalogue with technical categories, Favorites, Recent, filters, quick slots, Details, and stable component identity.
8. **Evidence feedback** - clear feedback explaining what happened and why.
9. **Reproducible checkpoints** - quick retries without repeating unrelated work.
10. **Design Scouting Report** - known strengths, limits, unknowns, and blocked requirements before final validation.
11. **Mastery profile** - progress based on validated work and recorded practice evidence.
12. **Project Evolution Timeline** - reproducible history of important project changes and results.
13. **Working templates** - editable examples that already function.
14. **Engineering Blueprints** - reusable versioned subsystems with dependencies, provenance, and stale-result handling.
15. **Creator modules** - reusable objectives, faults, instruments, budgets, timers, and validation triggers.
16. **Creator playtest** - fast testing using the exact same validators as normal gameplay.
17. **Repair-state gameplay** - explicit faults, diagnosis, repair, and final re-validation.
18. **Inspection markers** - saved measurement and repair locations across linked views.
19. **Large-project streaming and LOD** - presentation can scale without weakening engineering state.
20. **2D Lite + full presentation** - same engineering truth, different visual cost.
21. **High-quality 3D inspection** - added only after component identity and physical models are trustworthy.
22. **Future safe scripting** - only after deterministic replay, permissions, provenance, and security rules are complete.

## The rule that never changes

The game layer can teach, guide, score, animate, recommend, stream, generate presentation assets, and present.

It cannot decide engineering truth.

A failed circuit stays failed. An unknown result stays unknown. A visual effect, score, difficulty level, creator script, blueprint, procedural model, progression system, inventory card, or unloaded visual region can never turn an invalid engineering result into a pass.

## Development order

The research does not change the current engineering priority order, but it sharpens the gameplay work inside that order:

1. bulletproof the build and test pipeline;
2. lock the minimum playable loop: **Place -> Connect -> Power -> Test -> Diagnose -> Repair -> Validate**;
3. formalize the engineering-truth and component-data model;
4. make the responsive 2D workbench and catalogue-driven component library genuinely enjoyable to control;
5. add placement/connection previews, safe cancel, undo/redo, fast retest, readable feedback, and accessibility before broad visual expansion;
6. add instruments, practice, contracts, checkpoints, scouting reports, project timelines, blueprints, repair-state systems, and large-project navigation only after the basic loop is smooth;
7. add creator tools and verified high-quality presentation in dependency order;
8. add high-fidelity 3D only after the core gameplay already feels good.

The research gives FormFactor a clearer destination. It does not justify skipping the engineering foundation required to reach it safely.

## Detailed research

- [Gameplay Feel Requirements](GAMEPLAY_FEEL_REQUIREMENTS.md)
- [Gameplay, Graphics, Presentation, and UX Benchmarks](GAMEPLAY_UX_BENCHMARKS.md)
- [GTA and Open-World Design Research](GTA_OPEN_WORLD_RESEARCH.md)
- [Fortnite and Roblox Creator-System Research](FORTNITE_ROBLOX_CREATOR_RESEARCH.md)
- [MLB The Show Design Research](MLB_THE_SHOW_DESIGN_RESEARCH.md)
- [NBA 2K, Spore, Space Engineers, and Extreme-Scale World Research](NBA2K_SPORE_SPACE_SCALE_RESEARCH.md)
- [Inventory and Component Library Design](INVENTORY_COMPONENT_LIBRARY_DESIGN.md)
