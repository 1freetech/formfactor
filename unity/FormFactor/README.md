# FormFactor Unity 3D Front End — Update 1.06

This folder is the new production 3D game-layer project for **FormFactor**.

- Engine: **Unity 6.3 LTS**
- Pinned editor: **6000.3.15f1**
- Game-layer language: **C#**
- Engineering-truth backend: the existing **C++ FormFactor core** remains authoritative.

## Run it

1. Install Unity Hub.
2. Install Unity **6000.3.15f1 (Unity 6.3 LTS)** with Windows or Linux build support as needed.
3. In Unity Hub choose **Add project from disk** and select `unity/FormFactor`.
4. Open `Assets/Scenes/FormFactorWorkbench.unity`.
5. Press **Play**.

The scene intentionally contains no hand-authored objects. `FormFactorWorkbench3D` creates the first 3D workbench, board, lighting, camera, components, placement ghost, grid, and controls at runtime.

## Current 3D controls

- Left click empty board: place the selected component.
- Left click + drag a component: move it.
- Right click one component, then another: add a visual connection.
- `1`–`6`: select Resistor, Capacitor, LED, Chip, Power, Connector.
- `Shift` + Arrow: move the focused component one grid step.
- `Ctrl+D`: duplicate the focused component into the nearest open location.
- `E`: Pick Same Part.
- `X`: disconnect the focused component without deleting it.
- `Delete` / `Backspace`: remove the focused component.
- `Ctrl+Z`: undo.
- `Ctrl+Y`: redo.
- `V`: ask for engineering validation status.
- Middle-mouse drag: orbit the camera.
- Mouse wheel: zoom.

## Engineering truth

The Unity code **fails closed** until the native C++ bridge is connected. Pressing `V` cannot invent a passing circuit. It reports the engineering result as unknown while the authoritative C++ core is unavailable to the Unity front end.

The next bridge step is to expose a small stable C ABI from the existing C++ core, build it as a native library for each target platform, and call that API from C# using P/Invoke. The game layer will send project state to the core and render only the core's returned result.

## Why Unity + C#

The 2026 Coursera scan behind Update 1.06 found Unity + C# to be the strongest recurring 3D game-development pairing by course representation and enrollment. The engine choice is documented in `docs/UNITY_CSHARP_ENGINE_DECISION.md`.
