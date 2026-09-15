# Third-party presentation dependencies

This file tracks open-source software and asset sources used or evaluated for FormFactor presentation work.

## Shipping in the visual lab

### raylib

- Repository: https://github.com/raysan5/raylib
- Version: 6.0
- License: zlib/libpng
- Use: 3D renderer, camera, input, model drawing, picking, and visual-lab window
- Status: used by `experiments/raylib_visual_lab`

## Approved for integration testing

### Dear ImGui

- Repository: https://github.com/ocornut/imgui
- License: MIT
- Intended use: component browser, inspector, codex, validation panels, search, tables, and technical tool UI
- Status: researched; not yet shipped in the FormFactor playable build

### rlImGui

- Repository: https://github.com/raylib-extras/rlImGui
- License: zlib
- Intended use: bridge Dear ImGui into the raylib visual lab
- Status: researched; not yet shipped in the FormFactor playable build

## Approved source for licensed geometry research

### KiCad 3D package model sources

- Repository: https://github.com/KiCad/kicad-packages3D-source
- License: CC-BY-SA 4.0 with KiCad library exception
- Intended use: real electronic package geometry for specific verified component packages
- Status: researched; no KiCad model files are redistributed by this branch yet

Before a KiCad model is added, record the exact source revision, original path, package identity, attribution, conversion steps, scale, orientation, and runtime destination.

## Architecture references only

### Prune

- Repository: https://github.com/deanblackborough/Prune
- License: MIT
- Use: editor/runtime architecture reference only

### SimpleRenderEngine

- Repository: https://github.com/mortennobel/SimpleRenderEngine
- License: MIT
- Use: SDL/OpenGL/camera/mesh/ImGui architecture reference only

## Rule

A dependency or asset does not enter a FormFactor release merely because it is open source. Its exact source, version, license, and intended use must be recorded first.
