# FormFactor 1.125 — Open-source C++ 3D/UI and electronics reference

Research date: 2026-09-18.

## C++ rendering and UI shortlist

FormFactor should not replace its working Godot client with a second renderer just because a repository is popular. The useful source patterns are separated by job:

| Project | Approx. stars | License | Best FormFactor use |
| --- | ---: | --- | --- |
| Dear ImGui | 76.2k | MIT | C++ engineering/debug UI, inspectors, tables, dockable tooling |
| Google Filament | 20.5k | Apache-2.0 | physically based material and lighting reference for realistic component presentation |
| bgfx | 17.5k | BSD-2-Clause | renderer abstraction reference if FormFactor later needs a native C++ multi-backend viewer |
| Open3D | 14.0k | BSD-3-Clause | mesh/point-cloud processing and scientific 3D utilities rather than the main game renderer |

Sources:
- https://github.com/ocornut/imgui
- https://github.com/google/filament
- https://github.com/bkaradzic/bgfx
- https://github.com/isl-org/Open3D

The 1.125 implementation uses the practical ideas without copying a second rendering engine into the game: layered PBR-style materials, component-specific primitive assemblies, a cleaner immediate interaction surface, and a C++ visual-geometry catalog that keeps rendering metadata separate from engineering truth.

## Electronics geometry and schematic truth references

Current KiCad 10 libraries are the strongest open engineering reference for FormFactor's component presentation. KiCad maintains separate official repositories for schematic symbols, PCB footprints, rendered 3D packages, and the 3D-model source.

- https://www.kicad.org/libraries/download/
- https://gitlab.com/kicad/libraries/kicad-symbols
- https://gitlab.com/kicad/libraries/kicad-footprints
- https://gitlab.com/kicad/libraries/kicad-packages3d
- https://gitlab.com/kicad/libraries/kicad-packages3d-source

KiCad library assets are CC BY-SA 4.0. FormFactor 1.125 does not copy the KiCad library collection or embed KiCad image files. Instead, it draws common IEEE/IEC/KiCad-style schematic shapes procedurally and uses independently authored primitive 3D assemblies. This avoids blurry JPEG symbols, allows infinite scaling, and keeps licensing/provenance clear.

## What changed in 1.125

The blank PCB no longer carries inherited 3D version/silkscreen text or generic starter pads. Placed components no longer print floating reference text onto the board. The component catalog no longer displays ASCII-art symbols.

The 3D component layer now distinguishes axial resistors, rotary potentiometers, ceramic disc capacitors, aluminum electrolytics, inductors, axial diodes and Zeners, through-hole LEDs, TO-92 BJTs, TO-220 MOSFETs, DIP logic/op-amp/comparator packages, tabbed regulators, cartridge fuses, switches, relays, headers, test points, sensors, buzzers, terminal blocks, and ground studs.

The live schematic mirror now uses vector symbols for all 25 current component families. Exact manufacturer package dimensions remain UNKNOWN unless a verified component source supplies them. The C++ geometry catalog intentionally stores normalized display proportions, not invented mechanical measurements.
