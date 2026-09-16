# FormFactor CAD Workbench

FormFactor 1.11 adds a real parametric CAD path to the existing PCB training/build lab. The first implemented object is a rectangular PCB/mechanical mounting plate with an optional four-hole pattern. The model can be changed through numeric dimensions or a small plain-language input, previewed as live 3D CSG inside Godot, saved as deterministic OpenSCAD source, exported to STL when OpenSCAD is installed, and exported to STEP when FreeCADCmd is installed.

## What is implemented

The in-game **CAD // PARAMETRIC** panel contains a feature tree, a dimension inspector, a text input, a live 3D preview, generated OpenSCAD source, and export controls. The initial feature tree contains one base plate and four subtractive mounting holes. Turning holes off rebuilds the model as a single solid. Invalid geometry such as a hole inset smaller than its radius is blocked before preview/export.

A dependency-free developer bridge lives at `tools/cad/formfactor_cad.py`. It uses the same model idea outside the game and supports deterministic source generation plus optional external-engine export.

```bash
# Generate OpenSCAD source without needing OpenSCAD installed.
python3 tools/cad/formfactor_cad.py \
  --prompt "120x80x2 mm plate, 4 holes diameter 3.2mm, inset 5mm" \
  --engine source \
  --output build/cad/plate.scad

# Export STL when OpenSCAD is installed and available on PATH.
python3 tools/cad/formfactor_cad.py \
  --prompt "120x80x2 mm plate, 4 holes diameter 3.2mm, inset 5mm" \
  --engine openscad \
  --output build/cad/plate.stl

# Export STEP when FreeCADCmd/freecadcmd is installed and available on PATH.
python3 tools/cad/formfactor_cad.py \
  --prompt "120x80x2 mm plate, 4 holes diameter 3.2mm, inset 5mm" \
  --engine freecad \
  --output build/cad/plate.step
```

The plain-language parser is intentionally narrow and deterministic. It understands `width x depth x thickness`, hole diameter, inset, four-hole requests, and solid/no-hole requests. It does not pretend to be a general unrestricted natural-language CAD system yet.

## Upstream projects used

The upgrade was built from four open-source references supplied for FormFactor:

- **pascalorg/editor — MIT.** FormFactor adopts the editor pattern of separating scene/feature hierarchy, selection, direct editing controls, and an inspector instead of hiding geometry behind one opaque button. No Pascal web runtime is bundled in the Godot executable.
- **earthtojake/text-to-cad — MIT.** FormFactor adopts the prompt-to-CAD workflow, explicit artifact handoff, and source-first approach. The current local implementation is deliberately smaller: validated plate parameters with SCAD/STL/STEP outputs rather than claiming all CAD/CAE/CAM skills.
- **FreeCAD/FreeCAD — LGPL-2.1.** FormFactor generates a small FreeCAD Python macro and runs `FreeCADCmd` as an optional separate process to create STEP. FreeCAD source code is not copied into or statically linked with FormFactor.
- **openscad/openscad — GPL-2.0-or-later.** FormFactor generates `.scad` text and optionally runs the installed `openscad` executable as a separate process to create STL. OpenSCAD source code is not copied into or linked with FormFactor.

This structure makes the CAD engines useful immediately while keeping their licensing boundaries visible and reviewable. The repository should retain this separation unless a future licensing review explicitly chooses a different integration model.

## Validation

`tests/test_cad_bridge.py` checks deterministic source generation, text parsing, FreeCAD macro generation, invalid-geometry rejection, and artifact/manifest creation without requiring either external engine.

`godot/scripts/validate_cad.gd` loads the actual playable scene headlessly and checks that the CAD panel initializes, text changes the inspector values, the embedded CSG preview rebuilds correctly, four-hole and solid modes work, and deterministic OpenSCAD CSG source is present. The normal GitHub validation workflow runs both tests.

## Engineering-truth boundary

The CAD Workbench is a geometry authoring and export subsystem. A shape appearing in the preview or exporting successfully does **not** prove electrical correctness, PCB manufacturability, mechanical strength, thermal safety, regulatory compliance, tolerancing, or fit against a real sourced part. Those claims remain subject to FormFactor's engineering-truth rules and to future implemented gates with explicit evidence.
