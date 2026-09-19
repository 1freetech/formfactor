# FormFactor Update 1.06

FormFactor now has a clearer game technology plan. We reviewed 20 common game-development languages and kept five that each solve a different problem. **C++** stays in charge of the real engineering rules. **C# with Unity 6.3 LTS** is the main 3D game layer. **Python** keeps visual component data in sync. **GDScript with Godot 4.7.2** gives us a fast open-source place to test controls and gameplay ideas. **TypeScript** gives us a small browser tool for looking at component and validation data.

The Unity side now has a real FormFactor project, a 3D Workbench scene, the newer building controls, and a safety bridge that says **UNKNOWN** when the C++ engineering core is not connected. The Godot prototype has a simple 3D board with placement, selection, Pick Same Part, duplicate, nudge, undo, and camera zoom. The Python visual pipeline makes the same visual-only component file for both Unity and Godot, so colors, shapes, and display sizes can stay together instead of slowly becoming different.

The browser inspector is written in TypeScript and is only for viewing and debugging data. It cannot decide whether a circuit is correct. CI now checks the new language layers in a separate job, while the existing C++ tests keep running as before. Old validation runs on the same branch are cancelled when a newer update arrives, which keeps the build queue cleaner. The older SDL/C++ Workbench is still in the repo while the new 3D frontend grows.

## Important links

- Five-language research and roles: `docs/OPEN_SOURCE_LANGUAGE_STACK.md`
- Unity/C# engine decision: `docs/UNITY_CSHARP_ENGINE_DECISION.md`
- Unity project: `unity/FormFactor/`
- Godot prototype: `godot/FormFactorPrototype/`
- Python visual pipeline: `tools/visual_pipeline/`
- TypeScript inspector: `web/formfactor-inspector/`

## Engineering truth rule

The graphics, game controls, prototype, Python tools, and browser inspector can make FormFactor easier to use and better to look at. They cannot invent engineering facts. Only the authoritative C++ core can produce an engineering PASS. Missing core evidence stays **UNKNOWN**.
