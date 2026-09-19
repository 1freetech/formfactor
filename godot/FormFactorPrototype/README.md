# FormFactor Godot Open Prototype

This is the fast open-source gameplay and interaction prototype for **FormFactor**.

- Engine: **Godot 4.7.2 stable**
- Language: **GDScript**
- Role: fast experiments for controls, camera feel, placement, selection, accessibility, and Workbench UX
- Production 3D frontend: `unity/FormFactor/` using Unity + C#
- Engineering truth: the C++ core in `src/` and `include/formfactor/`

## Run

Open this folder as a Godot project:

`godot/FormFactorPrototype/`

Then press **Run Project**.

The scene creates its board and prototype components from GDScript so interaction ideas can be tested quickly without waiting on production assets.

## Prototype controls

- Left click empty board: place selected component
- Left click component: select it
- 1–5: select LED, resistor, capacitor, chip, or power
- E: Pick Same Part
- Ctrl+D: duplicate selected component without copying wires
- Shift+Arrow: move selected component one grid step
- Ctrl+Z: undo
- Mouse wheel: move the prototype camera closer or farther
- V: request validation

## Engineering-truth rule

The Godot layer cannot certify a circuit. Until its bridge is connected to the authoritative C++ core, `V` returns **UNKNOWN**, never PASS. A useful gameplay prototype is not engineering evidence.
