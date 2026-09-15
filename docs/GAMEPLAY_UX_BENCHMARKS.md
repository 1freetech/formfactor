# FormFactor Gameplay, Graphics, Presentation, and UX Benchmarks

Status: design contract for the production frontend. This document translates general lessons from highly rated games and accessibility guidance into original FormFactor requirements. It does **not** authorize copying proprietary code, art, audio, characters, dialogue, level designs, branding, or distinctive protected UI expression.

## Core playable loop

FormFactor's minimum playable engineering loop is:

**Place -> Connect -> Power -> Test -> Diagnose -> Repair -> Validate**

The player should always be able to see:

- the current goal;
- the current step;
- what changed after the last action;
- what failed or remains unknown;
- one clear next action.

Difficulty should come from engineering decisions, not confusing menus or hidden controls.

## Gameplay rules

1. **Fast, readable feedback.** Placement, wiring, instrument, and validation actions should update the interface immediately when the underlying result is available. Longer solver work must show a clear running state.
2. **Failure teaches.** A failed check names the violated rule, shows the evidence available from the engineering core, highlights the affected object, and gives one useful repair direction without inventing the answer.
3. **Progress through mastery.** Early challenges teach one engineering idea at a time. Later challenges combine skills already introduced.
4. **Multiple valid solutions.** When the engineering core accepts several designs, the game must not force one cosmetic or scripted answer.
5. **Sandbox stays open.** Guided lessons and challenges sit beside unrestricted experimentation.
6. **No fake wins.** Animation, score, or UI state can never turn an engineering failure or unknown result into a pass.

## Presentation rules

The interface should feel polished and tactile without hiding engineering information.

- Use strong visual hierarchy: objective first, active workspace second, selected-part information third, deep evidence on demand.
- Selected objects use a clear outline plus a text/reference label; never rely on color alone.
- Valid, warning, failed, unknown, and running states use icon/shape + text + color.
- Motion is short and functional: placement snap, connection confirmation, test start/finish, and validation result transitions.
- Decorative bloom, shake, particles, or animation must not make traces, pins, warnings, waveforms, measurements, or text harder to read.
- Realistic materials and component models are used only when asset identity and license are known. Visual-only models stay explicitly marked `visual-only`.
- 2D is the primary fast construction view. 3D is for inspection, orientation, assembly understanding, and presentation after authoritative geometry exists.

## Default workbench layout

**Top:** project state, objective, current step, save/run state.

**Left:** component library, search, categories, and filters.

**Center:** schematic/board workspace.

**Right:** selected component, value, package, pins, model status, key limits, and a **Details** path to provenance/evidence.

**Bottom:** action/validation timeline, warnings/errors, instrument output, and the primary next action.

The same component identity and selection remain linked across catalogue, schematic, PCB, instrument, and 3D views.

## Accessibility and usability baseline

Accessibility is part of the production UI contract, not a late add-on.

- Fully remappable keyboard and gamepad actions.
- Mouse, keyboard-only, and gamepad navigation for normal gameplay screens.
- On-screen prompts reflect the active bindings.
- Adjustable text scale and UI scale.
- High-contrast interface option.
- Critical information is never conveyed by color alone.
- Reduced-motion option.
- Haptics/vibration toggle and strength control where supported.
- Pointer/stick sensitivity controls where relevant.
- Pause and step controls for simulation/training where the underlying system supports them.
- Large, well-spaced interactive targets.
- Accessibility options are visible during first-run setup and remain available in Settings.

## Production frontend architecture

Use the current approved small-engine architecture:

- **formfactor_core**: authoritative engineering data, validation, simulation, provenance, and replay records.
- **SDL3**: windowing, keyboard/mouse/gamepad input, controller mappings, and platform event handling.
- **bgfx**: cross-platform rendering abstraction. Renderer/backend choice must not change engineering results.
- **Dear ImGui**: engineering panels, tools, inspectors, diagnostics, settings, and workbench UI. FormFactor owns and tests the player-facing interaction contract.
- **GLB/glTF 2.0**: optimized runtime 3D presentation assets.
- **STEP**: authoritative mechanical source format when available.

The temporary SDL2 FreeLab prototype remains a smoke-test harness. Do not sink major presentation effort into code intended to be replaced by the production frontend.

## Open-source dependency policy

Pin dependency versions when integration lands and ship notices required by each dependency and asset.

- bgfx: BSD 2-Clause.
- SDL3: zlib license.
- Dear ImGui: MIT license.

Third-party samples, shaders, fonts, controller databases, assets, and bundled subdependencies require their own license review.

## Implementation order

1. Keep the build/test pipeline reliable.
2. Lock the minimum playable loop in the engineering core and UI state model.
3. Lock engineering-truth/component-data contracts.
4. Build the responsive 2D workbench around the loop.
5. Add input remapping, UI scaling, high contrast, reduced motion, and full focus navigation before broad content expansion.
6. Add instrument feedback and replay.
7. Add verified high-fidelity physical assets and 3D inspection after package/footprint/model linkage is authoritative.
8. Optimize presentation after startup time, input latency, frame time, memory, and project-open time can be measured repeatably.

## Frontend acceptance tests

- A new player can complete the minimum loop without external documentation.
- Keyboard-only and gamepad-only users can reach every normal gameplay action in the loop.
- Rebinding an action changes its on-screen prompt.
- Increasing UI scale does not hide the primary action or validation state.
- Every failure state includes text and a non-color visual indicator.
- A long-running check has a visible running state and cannot be mistaken for success.
- Multiple engineering-valid solutions remain accepted.
- Changing a component, package, or value invalidates stale results visibly.
- Rendering/UI code cannot set engineering pass/fail results.
- 2D Lite and full presentation profiles use the same truth and validation gates.

## Research translation

The research pass found recurring strengths among highly reviewed games across different genres: responsive interaction, a satisfying repeatable loop, player agency, strong visual cohesion, meaningful variety, and presentation that makes actions feel tangible. Those are adopted here only as general design principles.

Accessibility guidance independently supports remappable controls, readable/resizable interfaces, high contrast, large interactive targets, and redundant status communication instead of color-only signals.

## Reference starting points

- Metacritic platform browse pages supplied/searched for PC, PS5, Xbox Series X/S, Nintendo Switch, and Nintendo Switch 2.
- Game Accessibility Guidelines: https://gameaccessibilityguidelines.com/
- bgfx documentation: https://bkaradzic.github.io/bgfx/
- SDL3 wiki: https://wiki.libsdl.org/SDL3/
- Dear ImGui repository/docs: https://github.com/ocornut/imgui
