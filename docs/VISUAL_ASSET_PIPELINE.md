# FreeLab visual asset and rendering contract

The current SDL2 workbench is a temporary smoke-test harness. Its rectangles and lines prove that the compiled project can open a native window through WSL; they are not the intended visual quality of PCBTech / FreeLab.

The project remains backend-first. Engineering data, solver results, dimensions, pin mappings, and validation state are authoritative. The renderer may visualize those records but must never invent electrical or mechanical facts.

## Target visual stack

When the backend milestones are ready for a production interface, use an open-source 3D engine rather than growing the SDL2 prototype into a custom renderer. The preferred target is **Godot 4** for the interactive 3D FreeLab workbench. Godot is open source under the MIT license and supports glTF 2.0 / GLB assets, physically based materials, lighting, cameras, picking, UI, and desktop deployment.

The C++ `pcbtech_core` library remains the engineering source of truth. The future Godot interface is a client of that core, not a replacement for it. Integration may use a narrow native binding or process/API boundary after the core data contracts stabilize.

Primary runtime 3D asset format: **GLB / glTF 2.0**.

Primary dimensionally accurate source format: **STEP** when available.

## Asset quality order

For each physical component/package, prefer assets in this order:

1. Manufacturer-provided STEP model for the exact package and variant.
2. A matching KiCad library 3D model whose footprint/package mapping has been verified.
3. A model built from authoritative mechanical drawings using an open-source CAD workflow such as FreeCAD / KiCad StepUp.
4. A clearly marked visual-only model when no authoritative dimensions exist.

Do not ship production components as generic rectangles when an accurate model is available. A visual approximation must remain explicitly `visual-only` and cannot satisfy mechanical-clearance or fabrication gates.

Real product photographs may be used as reference or catalogue-card imagery only when their license permits it. Photographs are not substitutes for dimensionally accurate board geometry.

## Component asset record

Every renderable catalogue entry should ultimately link these versioned records:

- manufacturer + exact part number
- functional category and tags
- schematic symbol
- PCB footprint
- pin/pad mapping
- simulation model and terminal mapping
- physical 3D model
- model orientation, scale, and offset relative to the footprint origin
- source URL/document revision
- asset revision/hash
- asset license and attribution requirements
- accuracy tier: verified, partial, or visual-only

The same stable component identity must be used in the component browser, schematic, board view, simulation view, and 3D workbench.

## 3D preparation pipeline

1. **Acquire** the exact STEP/VRML/source model and record provenance/license.
2. **Verify** package name, dimensions, pin count, pin-1 marker, body orientation, and footprint assignment against the authoritative datasheet.
3. **Normalize** origin, axes, scale, and board seating height without changing authoritative dimensions.
4. **Convert** the runtime copy to GLB/glTF while retaining the original source asset and metadata.
5. **Materialize** realistic package plastics, metals, ceramic bodies, solder, copper, silkscreen, solder mask, and board substrate using PBR materials.
6. **Align** the model to the verified footprint and test pad/pin correspondence.
7. **Generate** catalogue thumbnails from the same model used in the workbench so the user sees one consistent part.
8. **Validate** model bounds and placement before allowing the asset to be marked verified.
9. **Cache** optimized runtime meshes while preserving the full-detail source model.

KiCad supports STEP and VRML component models and current KiCad PCB exports include GLB output. This makes KiCad libraries and exports useful sources for the future asset pipeline while keeping the exact catalogue mapping under PCBTech control.

## Visual quality requirements

The final FreeLab workbench should support:

- physically plausible perspective and orthographic inspection cameras
- smooth pan/orbit/zoom and component selection
- dimensionally correct board/component placement
- realistic PBR materials and lighting
- anti-aliasing and high-resolution rendering
- component outlines and labels that remain readable over realistic geometry
- selectable copper, solder mask, silkscreen, substrate, vias, pads, and layers
- exploded/inspection views where useful
- accurate pin-1, polarity, and orientation markers
- linked highlighting between schematic, footprint, and 3D component
- quality levels / LOD so large boards remain responsive

Visual effects must never obscure warnings, unknown values, or validation failures.

## Backend-before-frontend gate

Do not spend major development time polishing 3D rendering until the following backend contracts are stable enough to drive it:

1. versioned component catalogue and functional taxonomy
2. symbol -> footprint -> physical-model -> simulation-model linkage
3. explicit pin/pad/model-terminal mapping
4. unit-aware component properties and search/filter data
5. electrical connectivity and ERC
6. deterministic digital/SPICE interfaces
7. board geometry, stackup, DRC, impedance, return-path, PDN, and decoupling contracts
8. thermal/power result interfaces
9. KiCad import/export model

After those are stable, replace the SDL2 smoke-test workbench with the high-fidelity 3D FreeLab frontend rather than polishing the primitive prototype.
