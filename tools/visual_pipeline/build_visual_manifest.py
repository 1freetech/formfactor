#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "tools" / "visual_pipeline" / "component_visuals.source.json"
OUTPUTS = [
    ROOT / "unity" / "FormFactor" / "Assets" / "FormFactor" / "Resources" / "visual_components.json",
    ROOT / "godot" / "FormFactorPrototype" / "data" / "visual_components.json",
]
ALLOWED_SHAPES = {"box", "cylinder", "sphere"}
HEX_COLOR = re.compile(r"^#[0-9A-F]{6}$")


def load_and_validate() -> dict:
    raw = json.loads(SOURCE.read_text(encoding="utf-8"))
    components = raw.get("components")
    if not isinstance(components, list) or not components:
        raise ValueError("components must be a non-empty list")

    seen: set[str] = set()
    normalized: list[dict] = []
    for index, component in enumerate(components):
        if not isinstance(component, dict):
            raise ValueError(f"component {index} must be an object")

        kind = component.get("kind")
        shape = component.get("shape")
        color = component.get("color")
        scale = component.get("scale")

        if not isinstance(kind, str) or not kind or kind != kind.upper():
            raise ValueError(f"component {index} kind must be non-empty uppercase text")
        if kind in seen:
            raise ValueError(f"duplicate component kind: {kind}")
        seen.add(kind)

        if shape not in ALLOWED_SHAPES:
            raise ValueError(f"{kind} has unsupported visual shape: {shape}")
        if not isinstance(color, str) or not HEX_COLOR.fullmatch(color):
            raise ValueError(f"{kind} color must be #RRGGBB uppercase hex")
        if not isinstance(scale, list) or len(scale) != 3:
            raise ValueError(f"{kind} scale must contain exactly 3 numbers")
        if any(not isinstance(value, (int, float)) or value <= 0 for value in scale):
            raise ValueError(f"{kind} scale values must be positive numbers")

        normalized.append(
            {
                "kind": kind,
                "shape": shape,
                "color": color,
                "scale": [round(float(value), 4) for value in scale],
            }
        )

    normalized.sort(key=lambda item: item["kind"])
    return {
        "schema": "formfactor-visual-component-manifest-v1",
        "visual_only": True,
        "warning": "Presentation metadata only. Never use this file for electrical validation.",
        "components": normalized,
    }


def encode(manifest: dict) -> str:
    return json.dumps(manifest, indent=2, sort_keys=False) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--check",
        action="store_true",
        help="fail if committed Unity/Godot manifests differ from generated output",
    )
    args = parser.parse_args()

    try:
        expected = encode(load_and_validate())
    except (OSError, json.JSONDecodeError, ValueError) as exc:
        print(f"ERROR: {exc}")
        return 1

    if args.check:
        failed = False
        for output in OUTPUTS:
            if not output.is_file():
                print(f"ERROR: missing generated visual manifest: {output.relative_to(ROOT)}")
                failed = True
                continue
            actual = output.read_text(encoding="utf-8")
            if actual != expected:
                print(f"ERROR: stale visual manifest: {output.relative_to(ROOT)}")
                failed = True
        if failed:
            return 1
        print("Cross-engine visual manifests: PASS")
        return 0

    for output in OUTPUTS:
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(expected, encoding="utf-8")
        print(f"wrote {output.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
