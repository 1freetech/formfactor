# FormFactor Open-Source Game Language Stack

Updated: September 15, 2026

There is no single authoritative internet ranking of programming languages for open-source games. FormFactor therefore uses a transparent proxy: major active open-source game engines/frameworks, their public GitHub adoption, and the primary language a game developer uses with them.

## Current proxy ranking

The ranking below is not a claim about every open-source game repository on the internet. It is a practical ecosystem signal for engine/language adoption.

1. **C++** — already FormFactor's engineering-core language. Godot's engine repository is primarily C++ and currently has roughly 117k GitHub stars. C++ also remains central to native engines such as Cocos2d-x.
2. **Rust** — Bevy is an open-source Rust game engine with roughly 48.2k GitHub stars.
3. **TypeScript** — Babylon.js is an Apache-2.0 open game/rendering engine written primarily in TypeScript with roughly 26.1k GitHub stars.
4. **GDScript** — Godot's official demo-project repository is primarily GDScript and has roughly 9.5k GitHub stars. GDScript is Godot's first-party gameplay scripting language.
5. **Lua** — LÖVE is a widely used open-source Lua game framework; its main repository has roughly 8.7k GitHub stars. Lua also has a long history as an embedded game scripting language.

C# remains important and is already represented in FormFactor through Unity. Stride is a free/open-source C# engine with roughly 7.8k GitHub stars.

## Sources

- Godot Engine: https://github.com/godotengine/godot
- Godot demo projects: https://github.com/godotengine/godot-demo-projects
- Bevy: https://github.com/bevyengine/bevy
- Babylon.js: https://github.com/BabylonJS/Babylon.js
- LÖVE: https://github.com/love2d/love
- Stride: https://github.com/stride3d/stride

Star counts are snapshots and will change. They are used only as one public adoption signal, not as a quality score.

## FormFactor implementation choice

FormFactor does **not** rewrite the same game independently in every language. Each language gets a narrow job.

### C++ — authoritative engineering core

C++ owns validated component data, circuit rules, simulation evidence, deterministic engineering gates, and final pass/fail/unknown decisions.

### C# / Unity — production 3D game frontend

Unity + C# is the production 3D presentation and gameplay layer. It handles scenes, camera, interaction, visuals, UI, and player-facing workflow. It consumes engineering results; it does not create them.

### GDScript / Godot — open-source interaction prototype lab

Godot + GDScript is used for fast open-source experiments with placement, camera feel, direct manipulation, accessibility, and Workbench UX. Successful interaction ideas can later be promoted into the Unity production frontend.

### Rust — deterministic native bridge helper

Rust is used where memory safety and deterministic native data handling are useful around the engine/core boundary. `rust/formfactor_bridge/` currently provides stable frontend snapshot ordering and fail-closed behavior when an authoritative C++ result is unavailable. It does not replace the C++ core.

### TypeScript — browser inspector and development tooling

TypeScript is used for browser-based component/snapshot inspection and lightweight developer tools in `web/formfactor-inspector/`. The inspector displays evidence and state without being part of the authoritative validation path.

### Lua — reserved, not implemented yet

Lua is not being added simply because it ranks highly. Its strongest FormFactor use would be future sandboxed creator/challenge scripting. FormFactor's current roadmap says user scripting comes only after deterministic replay, permissions, provenance, and security controls are complete, so adding Lua now would be premature.

## Non-negotiable rule

A frontend, prototype, web tool, or scripting layer may display, request, transform, or explain engineering evidence. It may never turn missing evidence into PASS. If the C++ core has not proven a result, every other language layer must preserve that result as UNKNOWN or FAIL as appropriate.
