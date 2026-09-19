# FormFactor Open Source 3D Visual Lab

This experiment tests a more game-like 3D presentation without changing the working SDL2 prototype on `main`.

## What it tests

- a real perspective 3D camera
- right-mouse orbit and mouse-wheel zoom
- WASD camera-target movement
- a 3D PCB work surface
- physical-looking resistor, capacitor, LED, DIP chip, battery source, and pin header
- schematic symbols shown beside the physical-part information
- mouse and keyboard component selection
- a tiny inspect-all-six-parts gameplay goal
- a cleaner game-style HUD for part learning

## Build

```bash
cmake -S experiments/raylib_visual_lab -B build-visual-lab
cmake --build build-visual-lab --parallel 2
```

Then run `formfactor_visual_lab` from the build folder.

The first configure downloads the pinned raylib 6.0 source release from GitHub through CMake FetchContent.

## Controls

- **Right mouse drag:** orbit the camera around the board.
- **Mouse wheel:** zoom in or out.
- **W A S D:** move the camera target across the board.
- **R:** reset the camera.
- **Tab / arrow keys:** cycle through components.
- **Mouse click on a part card:** inspect that component.
- **Esc:** exit.

## Why this is separate

The main FormFactor build already works on Windows and Linux. This lab lets us test a stronger visual stack before we touch the stable build pipeline. Anything that proves useful can be moved into the main workbench in small reviewed steps.

## Open-source dependency

The lab uses [raylib](https://github.com/raysan5/raylib), pinned to release `6.0`. raylib uses the zlib/libpng license. See the research note in `docs/OPEN_SOURCE_VISUAL_RESEARCH.md` for the other projects reviewed for FormFactor.
