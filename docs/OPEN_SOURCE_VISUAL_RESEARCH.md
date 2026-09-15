# Open-source visual and gameplay research for FormFactor

This note records open-source projects that can help FormFactor look and feel more like a real game while preserving the engineering-truth layer underneath it.

The rule for this research is simple: do not copy code or assets into FormFactor unless the license is clear and the source is recorded.

## 1. raylib — strongest low-risk 3D prototype candidate

Repository: https://github.com/raysan5/raylib

License: zlib/libpng.

Why it matters for FormFactor:

- simple 3D camera support
- model loading and 3D shapes
- shaders and post-processing
- Windows, Linux, macOS, Android, and HTML5 support
- small API that is good for prototypes and education

Current action:

The branch `experiment/visual-lab-raylib` contains a separate raylib 6.0 sandbox. It does not replace the main SDL2 workbench. The goal is to prove camera movement, true 3D depth, physical component presentation, HUD structure, and simple exploration gameplay before we decide what belongs in the main game.

## 2. Dear ImGui — best candidate for professional tool panels

Repository: https://github.com/ocornut/imgui

License: MIT.

Why it matters for FormFactor:

- mature inspector, panel, tree, table, popup, tooltip, and docking patterns
- keyboard and gamepad navigation support
- officially maintained SDL2 platform backend
- SDL2 renderer backend is available, so FormFactor can test ImGui without immediately abandoning SDL2

Possible FormFactor use:

- component inspector
- searchable codex
- schematic-properties panel
- validation and error panels
- inventory/category browser
- debug and engineering-truth views

Do not move the whole game into ImGui. Use it where a technical panel needs to be clear, searchable, and keyboard friendly.

## 3. KiCad 3D model libraries — best source for real component geometry

Repository reviewed: https://github.com/KiCad/kicad-packages3D-source

License: CC-BY-SA 4.0 with the KiCad library exception.

Why it matters for FormFactor:

- real electronics package geometry
- resistor, capacitor, connector, DIP, SMD, sensor, relay, transformer, and many other component families
- closer to the exact physical parts FormFactor is trying to teach

Important license boundary:

KiCad says designs using the library data do not automatically become CC-BY-SA. However, redistributing the KiCad libraries or portions of them as a collection requires the same license and attribution information to travel with that collection.

Recommended FormFactor approach:

- do not blindly copy the entire KiCad model library into FormFactor
- create a small asset manifest that names the exact model, source URL, source revision, and license
- fetch only models the game actually needs
- keep attribution and license text beside redistributed model files
- prefer STEP/IGES for geometric correctness when practical and converted render formats for runtime performance

## 4. Prune — useful editor/runtime architecture reference

Repository: https://github.com/deanblackborough/Prune

License: MIT.

Why it matters for FormFactor:

Prune is built around an editor and runtime living together instead of separating "edit mode" and "play mode" completely. That pattern fits FormFactor well because building the circuit is also the gameplay.

Ideas worth adapting, not blindly copying:

- one runtime shell with scene-specific tools
- editor controls that remain available while the simulation runs
- camera state that is separate from actual design state
- shared panels and inspectors around different scene types

## 5. SimpleRenderEngine — useful SDL2 + OpenGL reference

Repository: https://github.com/mortennobel/SimpleRenderEngine

License: MIT.

Why it matters for FormFactor:

It demonstrates a compact C++ renderer using SDL2, OpenGL, camera support, meshes, shaders, textures, shadows, and Dear ImGui. It is useful evidence that FormFactor does not necessarily need to throw away SDL2 in order to grow into a stronger 3D renderer.

## Recommended path

### Phase A — experiment without destabilizing main

Use the raylib visual lab to answer these questions:

1. Does orbit/zoom make PCB work easier to understand?
2. Do realistic component shapes improve learning enough to justify 3D assets?
3. Which HUD panels should remain visible while building?
4. How much schematic information should be visible beside the physical board?
5. Can the player understand what to do without reading a manual first?

### Phase B — bring only proven ideas into main

Move successful ideas into the stable workbench one at a time:

- hover and selection feedback
- camera controls
- better component geometry
- real model loading
- polished inspector/codex panels
- placement previews and snapping
- live schematic synchronization
- small training objectives and progression

### Phase C — exact models and stronger presentation

After the renderer path is proven:

- add a licensed component-asset manifest
- begin with a tiny verified set of real component models
- add PBR-style materials, shadows, and lighting only if they stay fast and clear
- keep engineering state separate from the renderer so visual polish can never create false electrical claims

## Source policy

For every imported dependency or asset, record:

- repository or source URL
- exact tag or commit
- license
- files actually used
- modifications made by FormFactor

Permissive dependencies such as MIT and zlib are preferred for code. Share-alike assets can still be used when their attribution and redistribution requirements are kept explicit.
