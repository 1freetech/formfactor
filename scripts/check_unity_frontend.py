#!/usr/bin/env python3
from pathlib import Path
import json
import sys

ROOT = Path(__file__).resolve().parents[1]
UNITY = ROOT / "unity" / "FormFactor"

required = [
    UNITY / "ProjectSettings" / "ProjectVersion.txt",
    UNITY / "ProjectSettings" / "EditorBuildSettings.asset",
    UNITY / "Packages" / "manifest.json",
    UNITY / "Assets" / "Scenes" / "FormFactorWorkbench.unity",
    UNITY / "Assets" / "FormFactor" / "Runtime" / "FormFactorWorkbench3D.cs",
    UNITY / "Assets" / "FormFactor" / "Runtime" / "EngineeringCoreBridge.cs",
]

errors = []
for path in required:
    if not path.is_file():
        errors.append(f"missing required Unity file: {path.relative_to(ROOT)}")

if errors:
    for error in errors:
        print(f"ERROR: {error}")
    sys.exit(1)

version = (UNITY / "ProjectSettings" / "ProjectVersion.txt").read_text(encoding="utf-8")
if "m_EditorVersion: 6000.3.15f1" not in version:
    errors.append("Unity editor pin is not 6000.3.15f1")

try:
    manifest = json.loads((UNITY / "Packages" / "manifest.json").read_text(encoding="utf-8"))
except json.JSONDecodeError as exc:
    errors.append(f"Unity package manifest is invalid JSON: {exc}")
    manifest = {}

physics = manifest.get("dependencies", {}).get("com.unity.modules.physics")
if physics != "1.0.0":
    errors.append("Unity physics module is not explicitly enabled")

build_settings = (UNITY / "ProjectSettings" / "EditorBuildSettings.asset").read_text(encoding="utf-8")
if "Assets/Scenes/FormFactorWorkbench.unity" not in build_settings:
    errors.append("FormFactorWorkbench.unity is not the enabled Unity build scene")

workbench = (UNITY / "Assets" / "FormFactor" / "Runtime" / "FormFactorWorkbench3D.cs").read_text(encoding="utf-8")
for marker in [
    "RuntimeInitializeOnLoadMethod",
    "FormFactorWorkbench3D",
    "WouldOverlap",
    "DuplicateSelected",
    "PickSamePart",
    "DisconnectSelected",
    "Undo()",
    "Redo()",
    "IEngineeringCoreBridge",
]:
    if marker not in workbench:
        errors.append(f"Unity workbench is missing required marker: {marker}")

bridge = (UNITY / "Assets" / "FormFactor" / "Runtime" / "EngineeringCoreBridge.cs").read_text(encoding="utf-8")
for marker in [
    "EngineeringValidationState.Unknown",
    "UnavailableEngineeringCoreBridge",
    "VALIDATION REMAINS UNKNOWN",
]:
    if marker not in bridge:
        errors.append(f"Unity engineering bridge is missing fail-closed marker: {marker}")

if errors:
    for error in errors:
        print(f"ERROR: {error}")
    sys.exit(1)

print("Unity 6.3/C# frontend structure: PASS")
print("Engineering bridge fail-closed contract: PASS")
