# Update 1.06 — Unity + C# 3D Engine Decision

Updated: September 15, 2026

## Decision

FormFactor will use **Unity 6.3 LTS + C#** as the production 3D game-layer stack while retaining the existing C++ engineering core as the authoritative backend.

The existing SDL2/C++ workbench and custom 3D prototype are not deleted in Update 1.06. They remain usable during the transition and provide a fallback while the Unity front end is validated.

## Why this pairing won the Coursera scan

The request was to use the most strongly represented 3D game-development engine/language pairing found in Coursera coursework, provided the engine and language naturally match.

Current Coursera evidence strongly favors Unity + C#:

- **C# Programming for Unity Game Development Specialization** — University of Colorado System — about **107,500 learners enrolled**, 4-course series, focused specifically on C# and Unity.
  - https://www.coursera.org/specializations/programming-unity-game-development/
- **Game Design and Development with Unity Specialization** — Michigan State University — about **36,000 learners enrolled**, 5-course series, includes both 2D and 3D games, including a 3D first-person shooter and 3D platformer.
  - https://www.coursera.org/specializations/game-design-and-development
- **Game Design and Development 3: 3D Shooter** explicitly uses the Unity engine for a complete 3D game project.
  - https://www.coursera.org/learn/game-design-and-development-3
- **Unity 6 Game Development with C# Scripting Specialization** is a current 2026 course series centered on Unity 6 and C#.
  - https://www.coursera.org/specializations/packt-unity-6-game-development-with-c-sharp-scripting

For comparison:

- **C++ Programming for Unreal Game Development Specialization** currently shows about **16,000 learners enrolled** and uses C++ with Unreal.
  - https://www.coursera.org/specializations/cplusplusunrealgamedevelopment
- Coursera also has a modern **Godot 3D Game Development Masterclass**, but the surfaced course page did not provide a comparable enrollment count during the Update 1.06 scan.
  - https://www.coursera.org/learn/packt-intro-to-3d-game-development-masterclass-in-godot-msjjn

The Coursera signal therefore points most strongly to **Unity + C#** rather than mixing an engine with a language it is not primarily taught with.

## Why Unity 6.3 LTS

Unity identifies **Unity 6.3 LTS** as the current long-term-support branch and states that it is supported through December 2027. FormFactor pins the starter project to editor version `6000.3.15f1` rather than an alpha or beta build.

Sources:

- https://unity.com/releases/unity-6/support
- https://unity.com/releases/editor/whats-new/6000.3.15f1

## Licensing tradeoff

Unity is **not open-source software**. FormFactor's own source code remains open-source, but using the Unity front end introduces a proprietary engine dependency.

As of the Update 1.06 research:

- Unity Personal has a stated financial threshold of **US $200,000** over the most recent 12 months.
  - https://unity.com/pages/license-compliance
- Unity canceled its game Runtime Fee in September 2024.
  - https://unity.com/blog/unity-is-canceling-the-runtime-fee
- Unreal is free for game development until product revenue crosses its royalty threshold, after which its standard game royalty is generally 5% on lifetime gross revenue above the first US $1 million.
  - https://www.unrealengine.com/license
- Godot is MIT-licensed, free, open source, and royalty-free.
  - https://godotengine.org/license/

Unity wins Update 1.06 because the user explicitly prioritized the most strongly represented Coursera 3D game-development stack. If engine licensing later becomes a harder requirement than Coursera prevalence, Godot remains the cleanest open-source fallback.

## FormFactor architecture

The engine migration follows one non-negotiable rule:

> **Unity renders and handles game interaction. C++ decides engineering truth.**

The architecture is:

1. **Unity/C# game layer** — 3D workbench, camera, component interaction, visual feedback, accessibility, menus, animation, audio, tutorials, and future presentation systems.
2. **Stable native bridge** — a small C ABI exported by FormFactor's existing C++ core and called from C# through P/Invoke.
3. **C++ engineering core** — component data, circuit rules, solvers, validation, evidence, provenance, and all pass/fail/unknown engineering decisions.
4. **Fail-closed behavior** — if the bridge or core is missing, Unity reports `unknown`. It may never invent a pass.

## Update 1.06 implementation

The first Unity project is under `unity/FormFactor/`.

It already provides a procedural 3D workbench with:

- orbit and zoom camera;
- 3D PCB work surface and grid;
- resistor, capacitor, LED, chip, power, and connector component visuals;
- snapped placement ghost;
- overlap blocking;
- click-and-drag movement;
- visual wiring;
- undo/redo;
- precise grid nudge;
- duplicate;
- disconnect/re-wire;
- Pick Same Part;
- delete;
- a visible fail-closed engineering-core status.

This is the start of the production 3D front end, not a claim that the native C++ bridge has already been completed.
