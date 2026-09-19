# FormFactor Game Language Stack

Updated: September 15, 2026

There is no single authoritative worldwide ranking of "game programming languages." FormFactor therefore uses a reproducible mix of evidence instead of pretending one list is definitive:

- JetBrains' 2025 game-development survey for languages actually used on game projects;
- GitHub's current `game-engine` topic for open-source engine/framework activity by primary repository language;
- major active game engines and platforms for important game-specific languages that primary-repository statistics undercount, such as GDScript, Lua, GML, and Luau.

## Top-20 candidate pool reviewed

The first six have direct recent game-project usage figures from JetBrains: **C# 40%, C++ 29%, Python 19%, JavaScript 17%, Java 14%, TypeScript 10%**. GitHub's current open-source `game-engine` topic separately shows strong repository activity for C++, C, C#, JavaScript, Python, Java, TypeScript, Rust, and Go. Godot's growth makes GDScript materially important even though engine repositories themselves are mostly counted as C++.

The 20-language candidate pool reviewed for FormFactor is:

1. C#
2. C++
3. Python
4. JavaScript
5. Java
6. TypeScript
7. GDScript
8. C
9. Rust
10. Lua
11. Go
12. Kotlin
13. Swift
14. Objective-C
15. Dart
16. Haxe
17. GML / GameMaker Language
18. Luau
19. GPU shader languages such as HLSL / GLSL
20. Zig

The ordering after the directly measured leaders is intentionally not presented as false precision. The purpose of the pool is to make sure FormFactor considered the major current game-language ecosystems before adding another dependency.

## Sources

- JetBrains, State of Game Development 2025: https://lp.jetbrains.com/the-state-of-gamedev-2025/
- GitHub `game-engine` topic: https://github.com/topics/game-engine
- Godot Engine: https://godotengine.org/
- Godot repository: https://github.com/godotengine/godot
- Bevy: https://github.com/bevyengine/bevy
- Babylon.js: https://github.com/BabylonJS/Babylon.js
- LÖVE: https://github.com/love2d/love
- Roblox Luau: https://github.com/luau-lang/luau
- Haxe: https://github.com/HaxeFoundation/haxe

## The five languages FormFactor is actively using

FormFactor does **not** rewrite the same game five times. Each language must have a narrow job that makes the project simpler or more capable.

### 1. C++ — authoritative engineering core

C++ owns component data, electrical and physical rules, simulation evidence, deterministic engineering gates, and final pass/fail/unknown decisions. Nothing in a game engine is allowed to overrule it.

### 2. C# / Unity — production 3D game frontend

Unity + C# is the production 3D presentation and gameplay layer. It handles scenes, camera, component interaction, UI, effects, player-facing workflows, undo/redo, direct manipulation, and future production graphics. The Unity bridge fails closed when the C++ core is unavailable.

### 3. Python — shared content and visualization pipeline

Python is already part of FormFactor's repository tooling and is now explicitly used to keep frontend visual data synchronized. `tools/visual_pipeline/build_visual_manifest.py` validates one visual-only component source file and deterministically generates matching Unity and Godot manifests.

The Python pipeline is **presentation only**. It contains shape, color, and display scale. It must never contain or invent electrical ratings, safety claims, or validation results.

### 4. GDScript / Godot — open-source interaction prototype lab

Godot 4.7.2 + GDScript is the fast open-source proving ground for placement, selection, camera feel, nudge/duplicate controls, accessibility, and Workbench UX. Successful interaction ideas can be promoted into the Unity production frontend.

Godot validation currently returns **UNKNOWN** until connected to the authoritative C++ core.

### 5. TypeScript — browser/debug visualization

TypeScript powers `web/formfactor-inspector/`, a small browser inspector for frontend snapshots and engineering evidence. It is useful for component/catalog inspection, debugging, and future browser-side visualization without adding those tools to the main Unity executable.

The TypeScript UI uses DOM text APIs rather than constructing user-visible data through `innerHTML`, and it defaults missing engineering results to **UNKNOWN**.

## Why the other candidates are not active languages now

- **JavaScript:** TypeScript covers the same browser role with stronger type checks.
- **Java/Kotlin/Swift/Objective-C/Dart:** native mobile languages add little while Unity already targets the relevant platforms.
- **C:** the C++ core already covers native low-level work.
- **Rust:** useful and promising, but a second native systems layer is unnecessary while the C++ engineering core already owns that boundary.
- **Lua/Luau:** strong future candidates for sandboxed creator/challenge scripting, but only after permissions, deterministic replay, provenance, and security controls are ready.
- **Go:** useful for servers, but FormFactor does not yet need a separate backend service language.
- **Haxe/GML:** useful ecosystems, but they would duplicate gameplay/runtime responsibilities already covered by Unity and Godot.
- **HLSL/GLSL:** shader work will be added inside the production rendering pipeline when visual fidelity reaches that stage; it does not need to become a separate general gameplay stack today.
- **Zig:** promising native tooling, but no current FormFactor subsystem needs another low-level language.

## Non-negotiable engineering rule

A frontend, prototype, content tool, or web inspector may display, request, transform, or explain engineering evidence. It may never turn missing evidence into PASS. If the C++ core has not proven a result, every other language layer must preserve the result as UNKNOWN or FAIL as appropriate.
