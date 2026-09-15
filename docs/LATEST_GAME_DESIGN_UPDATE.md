# Latest FormFactor Game Design Update

Updated: September 15, 2026

This page is the short version of the latest FormFactor game-design direction. It combines the useful lessons from the game research without copying proprietary code, art, audio, characters, missions, branding, menus, broadcast packages, or protected level design.

## What FormFactor should feel like

FormFactor should be easy to start, satisfying to use, and deep enough to grow with the player.

The player should be able to:

- open FreeLab and experiment immediately;
- start a guided Engineering Contract when they want structure;
- practice one engineering skill without replaying a full mission;
- fail, understand why, and retry quickly;
- move between schematic, PCB, instruments, validation, and later 3D without losing context;
- build from working templates instead of always starting from an empty screen;
- eventually create and share safe custom engineering challenges;
- see clear feedback after important actions;
- improve through real engineering evidence instead of cosmetic points.

## What each game family adds

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

## Combined FormFactor game systems

The current design direction now includes these major game-facing systems:

1. **FreeLab sandbox** - unrestricted experimentation using the real engineering core.
2. **Engineering Contracts** - guided multi-stage jobs using the same project state as FreeLab.
3. **Engineering Practice Lab** - short focused drills for one skill at a time.
4. **Evidence feedback** - clear feedback explaining what happened and why.
5. **Reproducible checkpoints** - quick retries without repeating unrelated work.
6. **Mastery profile** - progress based on validated work and recorded practice evidence.
7. **Working templates** - editable examples that already function.
8. **Creator modules** - reusable objectives, faults, instruments, budgets, timers, and validation triggers.
9. **Creator playtest** - fast testing using the exact same validators as normal gameplay.
10. **2D Lite + full presentation** - same engineering truth, different visual cost.
11. **High-quality 3D inspection** - added only after component identity and physical models are trustworthy.
12. **Future safe scripting** - only after deterministic replay, permissions, provenance, and security rules are complete.

## The rule that never changes

The game layer can teach, guide, score, animate, recommend, and present.

It cannot decide engineering truth.

A failed circuit stays failed. An unknown result stays unknown. A visual effect, score, difficulty level, creator script, or progression system can never turn an invalid engineering result into a pass.

## Development order

The research does not change the current engineering priority order:

1. bulletproof the build and test pipeline;
2. lock the minimum playable loop: **Place -> Connect -> Power -> Test -> Diagnose -> Repair -> Validate**;
3. formalize the engineering-truth and component-data model;
4. build the responsive 2D workbench;
5. add feedback, instruments, practice, contracts, checkpoints, and accessibility;
6. add creator tools and verified high-quality presentation in dependency order.

The research gives FormFactor a clearer destination. It does not justify skipping the engineering foundation required to reach it safely.

## Detailed research

- [Gameplay, Graphics, Presentation, and UX Benchmarks](GAMEPLAY_UX_BENCHMARKS.md)
- [GTA and Open-World Design Research](GTA_OPEN_WORLD_RESEARCH.md)
- [Fortnite and Roblox Creator-System Research](FORTNITE_ROBLOX_CREATOR_RESEARCH.md)
- [MLB The Show Design Research](MLB_THE_SHOW_DESIGN_RESEARCH.md)
