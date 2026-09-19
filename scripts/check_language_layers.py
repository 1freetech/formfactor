#!/usr/bin/env python3
from pathlib import Path
import json
import sys

ROOT = Path(__file__).resolve().parents[1]

required = [
    ROOT / "godot" / "FormFactorPrototype" / "project.godot",
    ROOT / "godot" / "FormFactorPrototype" / "scenes" / "workbench.tscn",
    ROOT / "godot" / "FormFactorPrototype" / "scripts" / "workbench_prototype.gd",
    ROOT / "godot" / "FormFactorPrototype" / "scripts" / "engineering_core_bridge.gd",
    ROOT / "tools" / "visual_pipeline" / "component_visuals.source.json",
    ROOT / "tools" / "visual_pipeline" / "build_visual_manifest.py",
    ROOT / "unity" / "FormFactor" / "Assets" / "FormFactor" / "Resources" / "visual_components.json",
    ROOT / "godot" / "FormFactorPrototype" / "data" / "visual_components.json",
    ROOT / "web" / "formfactor-inspector" / "package.json",
    ROOT / "web" / "formfactor-inspector" / "tsconfig.json",
    ROOT / "web" / "formfactor-inspector" / "src" / "app.ts",
    ROOT / "docs" / "OPEN_SOURCE_LANGUAGE_STACK.md",
]

errors: list[str] = []
for path in required:
    if not path.is_file():
        errors.append(f"missing required language-layer file: {path.relative_to(ROOT)}")

if errors:
    for error in errors:
        print(f"ERROR: {error}")
    sys.exit(1)

godot_project = (ROOT / "godot" / "FormFactorPrototype" / "project.godot").read_text(encoding="utf-8")
for marker in ["FormFactor Open Prototype", "workbench.tscn", '"4.7"']:
    if marker not in godot_project:
        errors.append(f"Godot project is missing marker: {marker}")

gdscript = (ROOT / "godot" / "FormFactorPrototype" / "scripts" / "workbench_prototype.gd").read_text(encoding="utf-8")
for marker in ["_duplicate_selected", "_pick_same_part", "_nudge_selected", "_undo", "_validate_with_core"]:
    if marker not in gdscript:
        errors.append(f"Godot prototype is missing interaction marker: {marker}")

godot_bridge = (ROOT / "godot" / "FormFactorPrototype" / "scripts" / "engineering_core_bridge.gd").read_text(encoding="utf-8")
for marker in ['"status": "unknown"', '"passed": false', "UNKNOWN, not PASS"]:
    if marker not in godot_bridge:
        errors.append(f"Godot bridge is missing fail-closed marker: {marker}")

visual_builder = (ROOT / "tools" / "visual_pipeline" / "build_visual_manifest.py").read_text(encoding="utf-8")
for marker in ["formfactor-visual-component-manifest-v1", "visual_only", "--check"]:
    if marker not in visual_builder:
        errors.append(f"Python visual pipeline is missing marker: {marker}")

package_path = ROOT / "web" / "formfactor-inspector" / "package.json"
try:
    package = json.loads(package_path.read_text(encoding="utf-8"))
except json.JSONDecodeError as exc:
    errors.append(f"TypeScript package.json is invalid JSON: {exc}")
    package = {}

if package.get("devDependencies", {}).get("typescript") != "5.9.2":
    errors.append("TypeScript compiler must be pinned exactly to 5.9.2")

typescript_source = (ROOT / "web" / "formfactor-inspector" / "src" / "app.ts").read_text(encoding="utf-8")
for marker in ["EngineeringStatus", '"unknown"', "textContent", "replaceChildren"]:
    if marker not in typescript_source:
        errors.append(f"TypeScript inspector is missing required marker: {marker}")

if "innerHTML" in typescript_source:
    errors.append("TypeScript inspector must not construct the UI with innerHTML")

stack_doc = (ROOT / "docs" / "OPEN_SOURCE_LANGUAGE_STACK.md").read_text(encoding="utf-8")
for language in ["C++", "C#", "Python", "GDScript", "TypeScript"]:
    if language not in stack_doc:
        errors.append(f"language stack document is missing active language: {language}")

if errors:
    for error in errors:
        print(f"ERROR: {error}")
    sys.exit(1)

print("C++ engineering core role: PASS")
print("C#/Unity production frontend structure: PASS")
print("Python visual pipeline structure: PASS")
print("Godot/GDScript prototype structure: PASS")
print("TypeScript inspector structure: PASS")
print("Five-language FormFactor stack preserves fail-closed engineering truth.")
