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
    ROOT / "rust" / "formfactor_bridge" / "Cargo.toml",
    ROOT / "rust" / "formfactor_bridge" / "src" / "lib.rs",
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

rust_source = (ROOT / "rust" / "formfactor_bridge" / "src" / "lib.rs").read_text(encoding="utf-8")
for marker in ["canonical_snapshot", "EngineeringStatus::Unknown", "missing_core_never_becomes_pass"]:
    if marker not in rust_source:
        errors.append(f"Rust bridge is missing required marker: {marker}")

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

if errors:
    for error in errors:
        print(f"ERROR: {error}")
    sys.exit(1)

print("Godot/GDScript prototype structure: PASS")
print("Rust bridge structure: PASS")
print("TypeScript inspector structure: PASS")
print("All non-C++ frontend/tooling layers preserve fail-closed engineering truth.")
