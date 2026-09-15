# FormFactor presentation architecture

FormFactor will use mature open-source presentation tools while keeping engineering truth inside the FormFactor core.

## Core rule

The renderer, camera, UI, models, lighting, animation, and effects are presentation systems only.

They may show engineering results.

They may not decide engineering results.

Only the FormFactor core may decide whether a component record is trusted, whether a connection is legal, whether a circuit passes validation, whether a solver result is accepted, or whether export is allowed.

## Presentation stack

### Renderer and camera: raylib

Use raylib for the 3D scene, camera, picking, model drawing, lighting experiments, and normal game input while the new visual path is proven.

The current experiment is pinned to raylib 6.0.

Why:

- mature cross-platform C library
- simple perspective and orthographic cameras
- model and texture loading
- shaders
- mouse picking
- Windows and Linux support today, with mobile paths available later
- small enough to understand and replace if needed

raylib does not own component truth, electrical validation, net rules, or simulation state.

### Professional UI: Dear ImGui

Use Dear ImGui for tool-like parts of FormFactor:

- component browser
- part codex
- properties inspector
- validation results
- searchable error list
- engineering evidence and provenance
- debug views
- optional docking layouts

Do not build the physical board itself as an ImGui canvas. The board remains a real rendered scene.

When the raylib experiment needs ImGui, prefer the small zlib-licensed rlImGui integration layer rather than writing a custom bridge from scratch.

### Real component geometry: licensed electronics model libraries

Use real component models where the license and source are clear.

KiCad 3D libraries are a strong source for package geometry such as:

- resistors
- capacitors
- DIP and SMD IC packages
- connectors
- headers
- relays
- transformers
- sensors
- switches

Do not import an entire external asset library blindly.

Each redistributed model must have an entry in a FormFactor asset manifest containing:

- component identity
- source repository or URL
- exact source revision
- original file path
- license
- attribution text when required
- conversion steps
- FormFactor runtime file path
- geometry scale and orientation notes

## Engineering-to-presentation boundary

The core should expose a read-only presentation snapshot.

A snapshot may contain:

- stable component ID
- reference designator such as R1 or U3
- package identity
- transform on the board
- pin locations
- connection/net IDs
- selected state
- engineering status supplied by the core
- warnings supplied by the core
- simulation values supplied by the core

The snapshot must not ask the renderer to calculate electrical meaning.

Bad example:

`renderer decides LED is safe because wire is green`

Good example:

`core reports current limit passed; renderer displays green status`

## Ownership

### FormFactor core owns

- component records
- provenance and accuracy tiers
- pin mappings
- connection rules
- net topology
- ERC-style checks
- simulation input and accepted solver evidence
- manufacturing gates
- pass, fail, warning, and unknown states
- export eligibility

### Presentation layer owns

- camera
- lighting
- shadows
- meshes
- textures
- animation
- hover and selection visuals
- panel layout
- tooltips
- keyboard and mouse navigation
- sound
- visual effects

## Dependency rule

Every third-party dependency must be pinned to a known tag or commit before it becomes part of a release build.

Record:

- project name
- repository URL
- exact version or commit
- license
- files or modules used
- local modifications

Do not track floating `main`, `master`, or `latest` branches in release builds.

## Asset rule

Every third-party asset must have a source and license before it can enter the playable build.

No mystery models.

No copied marketplace assets without redistribution rights.

No geometry should be presented as an exact package unless the source, package name, and scale are known.

If the exact model is not available, the UI should label it as a prototype representation.

## Migration plan

1. Keep the current SDL2 prototype stable.
2. Prove raylib camera, picking, model loading, and interaction in the visual lab.
3. Add Dear ImGui to the visual lab for the codex and inspector.
4. Add a tiny licensed real-model set with complete provenance.
5. Feed the visual lab from a read-only FormFactor presentation snapshot.
6. Compare usability and build reliability with the SDL2 prototype.
7. Move only proven pieces into the stable game.

## Non-negotiable rule

Visual polish can improve understanding, but it can never override engineering truth.
