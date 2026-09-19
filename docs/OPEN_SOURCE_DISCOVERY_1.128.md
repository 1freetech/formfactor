# FormFactor 1.128 open-source discovery

## Repositories reviewed

1. [Godot demo projects](https://github.com/godotengine/godot-demo-projects) — MIT license.
   - Useful pattern: the official Material Testers demo uses real 3D material response, antialiasing, anisotropic filtering, and higher-quality shadows to make shape and surface changes easy to read.
   - FormFactor implementation: original Godot 4.7.2 code adds a separate visual PCB core, bottom copper, top solder mask, copper edge rails, plated-hole annuli, fiducials, and a real shadow-casting rim light. No demo source or assets were copied.
2. [Godot Orbit Camera](https://github.com/unovafr/Godot-Orbit-Camera) — GNU LGPL v3.
   - Useful pattern: keep orbit and zoom centered on a clear anchor so inspection stays predictable.
   - FormFactor comparison: the existing 3D workbench already has anchored orbit, wheel zoom, and component focus. No LGPL code was imported in this update.
3. [Godot Alignment Tool](https://github.com/zaevi/godot-alignment-tool) — MIT license.
   - Useful pattern: clear spatial alignment feedback matters in 2D and 3D editors.
   - FormFactor comparison: the current snap grid and placement preview already cover the immediate need. No dependency or code was imported.

## Truth boundary

All 1.128 board layers, plated holes, fiducials, lighting, and materials are visual-only presentation geometry. They do not claim a real stackup, copper weight, finish, drill size, impedance, clearance, thermal behavior, safety, or manufacturability. The C++ validators and trusted solver evidence remain authoritative.
