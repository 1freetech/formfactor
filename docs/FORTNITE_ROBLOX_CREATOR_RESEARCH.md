# Fortnite and Roblox creator-system research for FormFactor

Status: design research and license guardrails. This document extracts general creator-platform lessons that are useful to FormFactor. It does **not** authorize copying Fortnite or Roblox assets, UI artwork, maps, characters, branding, proprietary engine code, protected game content, or platform-specific services.

## Why these platforms matter to FormFactor

Fortnite Creative / UEFN and Roblox are strong references for a different problem than traditional single-player games: they make creation itself part of the product. Both reduce the distance between **build -> test -> change -> play again**, expose reusable building blocks, provide templates, and support experiences that can grow from simple examples into complex systems.

For FormFactor, the useful translation is not a generic user-generated-content platform. It is a future **engineering challenge creator** that lets users build safe, versioned training scenarios on top of the same engineering-truth core used by FreeLab.

## Fortnite / UEFN lessons

Epic's official Fortnite documentation describes two complementary creation layers: Fortnite Creative for direct sandbox building and Unreal Editor for Fortnite (UEFN) for deeper editing and scripting. Template islands provide working examples that can be opened, changed, and used as tutorials or test environments. Devices provide reusable event-driven gameplay behavior, while Verse can define custom rules and behaviors.

Useful FormFactor translations:

- **Templates that already work.** Ship editable starter projects for common tasks such as LED current limiting, voltage-divider measurement, decoupling placement, connector wiring, continuity diagnosis, and simple digital logic.
- **Reusable engineering devices.** Treat instruments, fault injectors, checkpoints, objectives, measurement targets, scoring rules, and validation triggers as reusable scenario modules instead of hard-coding every lesson.
- **Build/test loop stays short.** A creator should be able to edit a contract, launch it, test the exact player flow, return to editing, and repeat without rebuilding unrelated project state.
- **Simple first, advanced later.** Provide a no-code/low-code contract editor for normal training scenarios, while leaving room for a bounded scripting layer when the core security and replay model are ready.
- **Editable examples teach the system.** Templates should demonstrate how a feature works while remaining safe to modify.
- **Creator UI mirrors the player model.** The creator should work with the same components, validation states, instruments, and evidence that players see, rather than a disconnected authoring model.

## Roblox lessons

Roblox Studio combines building, scripting, testing, device emulation, and publishing in one environment. Its official documentation emphasizes editable templates, drag-and-drop world building, testing across screen sizes/form factors, customizable UI, analytics-driven iteration, and intelligent client streaming for large experiences.

Useful FormFactor translations:

- **One application for build + test.** Do not require a separate authoring program for basic Engineering Contracts. FreeLab and the contract editor should share the same project and component records.
- **Device/form-factor preview.** Add viewport presets and UI checks for common desktop resolutions and later handheld/lower-power layouts so the workbench can be tested before release.
- **Streaming by need.** Large 3D component models, textures, environment assets, waveform history, and optional lesson media should load only when needed. Engineering data and validation records remain authoritative and available independently.
- **Customizable but bounded UI.** Scenario authors can choose which panels, instructions, instruments, and objectives are visible, but cannot hide mandatory safety/validation information or override truth states.
- **Templates lower the entry barrier.** New users should be able to open a working circuit or board challenge immediately, inspect it, modify it, and learn by experimentation.
- **Creator analytics should measure friction, not manipulate outcomes.** Future local/opt-in metrics can measure completion time, retry points, abandoned steps, UI navigation, and common failure categories to improve tutorials. Metrics never alter engineering pass/fail results.

## Open-source / reusable technology notes

### Luau

Luau, the scripting language associated with Roblox, is open source under the MIT license. Its upstream project describes it as a small, fast, embeddable language based on Lua with gradual typing. It is implemented in C++ and exposes compiler, analysis, bytecode, and VM components.

**FormFactor position:** Luau is a possible future candidate for sandboxed scenario scripting because its license is permissive and its embedding model fits a C++ application. It is **not adopted yet**. Before any scripting engine is integrated, FormFactor must define a strict capability model so scripts cannot bypass engineering validation, fabricate solver output, access arbitrary files/network resources, or mutate authoritative records outside permitted APIs.

### Fortnite / Unreal technology

UEFN and Fortnite are excellent design references, but Fortnite content and Unreal/UEFN technology are not treated as open-source dependencies for FormFactor. Do not copy Fortnite assets, Verse examples into production without checking their applicable terms, or depend on Fortnite-specific services.

### Roblox platform technology

Roblox Studio and the Roblox engine are design references, not dependencies. Roblox platform code, services, assets, UI, and proprietary engine behavior are not copied into FormFactor. Only separately published open-source projects with compatible licenses may be evaluated for reuse.

## FormFactor systems to add to the long-term design

1. **Engineering Contract templates** — editable working examples with explicit objectives, allowed components, constraints, expected measurements, and validation gates.
2. **Scenario modules** — reusable objective, checkpoint, fault, instrument, timer, budget, scoring, and evidence modules connected by explicit events.
3. **Playtest mode** — launch the current contract exactly as a learner would experience it, then return to editing without losing author state.
4. **Safe creator scripting** — future optional layer, capability-limited and deterministic/replayable where possible. Luau can be evaluated as a candidate; no script may set engineering truth.
5. **Form-factor preview** — test UI scale, navigation, readability, and input across multiple window/device profiles before a build is published.
6. **Streaming presentation layer** — load heavy visual/media assets on demand while preserving project, validation, and replay state in the core.
7. **Shareable content packs** — versioned contracts/templates/components only after project formats, licenses, signatures, provenance, and trust policies are stable.
8. **Creator diagnostics** — show missing dependencies, unsupported solver requirements, unavailable assets, invalid component mappings, and licensing/provenance blockers before a contract can be published.

## Acceptance rules

- A template can be opened, modified, saved as a separate project, and played without changing the original.
- Playtest mode uses the same validation APIs as normal FreeLab.
- Scenario modules can trigger presentation and objectives but cannot directly write pass/fail engineering results.
- Creator-authored faults are explicit in the scenario/replay record.
- A contract that requires an unavailable solver, model, or authoritative property cannot claim completion.
- UI/device preview cannot weaken validation rules.
- Streaming out a visual asset cannot remove or alter authoritative engineering state.
- Imported creator content must carry version, origin, license, and trust/provenance metadata before it can enter a trusted catalogue or public content pack.
- Any future scripting runtime defaults to least privilege and exposes only explicit FormFactor APIs.

## Sources

- Epic Games, **Get Started Creating in Fortnite**: https://dev.epicgames.com/documentation/fortnite/get-started-creating-in-fortnite
- Epic Games, **Template Island**: https://dev.epicgames.com/documentation/fortnite/template-island
- Epic Games, **Creating Gameplay with Devices in Fortnite**: https://dev.epicgames.com/documentation/fortnite/creating-gameplay-with-devices-in-fortnite
- Epic Games, **Programming with Verse in UEFN**: https://dev.epicgames.com/documentation/fortnite/programming-with-verse-in-unreal-editor-for-fortnite
- Roblox Creator Hub, **Experiences**: https://create.roblox.com/docs/experiences
- Roblox Creator Hub, **Roblox Studio**: https://create.roblox.com/docs/studio
- Roblox Creator Hub, **Performance Optimization**: https://create.roblox.com/docs/performance-optimization
- Roblox Creator Hub, **UI and UX Design**: https://create.roblox.com/docs/production/game-design/ui-ux-design
- Luau upstream project: https://github.com/luau-lang/luau
