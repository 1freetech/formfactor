#!/usr/bin/env python3
"""FormFactor CAD bridge.

Creates deterministic OpenSCAD and FreeCAD source for simple parametric PCB/mechanical
fixtures. Optional exports invoke installed OpenSCAD or FreeCAD executables as separate
processes; neither engine is vendored or linked into FormFactor.
"""

from __future__ import annotations

import argparse
import json
import re
import shutil
import subprocess
import sys
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Iterable


NUMBER = r"([0-9]+(?:\.[0-9]+)?)"


@dataclass(frozen=True)
class PlateModel:
    width_mm: float = 100.0
    depth_mm: float = 70.0
    thickness_mm: float = 2.0
    hole_diameter_mm: float = 3.2
    hole_inset_mm: float = 5.0
    hole_count: int = 4

    def validate(self) -> None:
        values = (
            self.width_mm,
            self.depth_mm,
            self.thickness_mm,
            self.hole_diameter_mm,
            self.hole_inset_mm,
        )
        if not all(v > 0.0 for v in values):
            raise ValueError("all dimensions must be greater than zero")
        if self.hole_count not in (0, 4):
            raise ValueError("the current FormFactor plate primitive supports 0 or 4 holes")
        if self.width_mm > 1000.0 or self.depth_mm > 1000.0 or self.thickness_mm > 100.0:
            raise ValueError("dimensions exceed the supported workbench range")
        if self.hole_count:
            radius = self.hole_diameter_mm / 2.0
            if self.hole_inset_mm <= radius:
                raise ValueError("hole inset must be larger than the hole radius")
            if self.hole_inset_mm + radius >= min(self.width_mm, self.depth_mm) / 2.0:
                raise ValueError("hole pattern does not fit inside the plate")
            if self.hole_diameter_mm >= min(self.width_mm, self.depth_mm):
                raise ValueError("hole diameter cannot be as large as the plate")

    def hole_centers(self) -> tuple[tuple[float, float], ...]:
        self.validate()
        if self.hole_count == 0:
            return ()
        x0, x1 = self.hole_inset_mm, self.width_mm - self.hole_inset_mm
        y0, y1 = self.hole_inset_mm, self.depth_mm - self.hole_inset_mm
        return ((x0, y0), (x1, y0), (x1, y1), (x0, y1))


def _first_float(pattern: str, text: str) -> float | None:
    match = re.search(pattern, text, flags=re.IGNORECASE)
    return float(match.group(1)) if match else None


def parse_prompt(prompt: str, base: PlateModel | None = None) -> PlateModel:
    """Parse a deliberately small, deterministic subset of plain-language CAD prompts."""
    model = base or PlateModel()
    text = " ".join(prompt.strip().lower().replace("×", "x").split())
    if not text:
        model.validate()
        return model

    values = asdict(model)

    dims = re.search(
        rf"{NUMBER}\s*x\s*{NUMBER}(?:\s*x\s*{NUMBER})?\s*(?:mm|millimeters?|millimetres?)?",
        text,
    )
    if dims:
        values["width_mm"] = float(dims.group(1))
        values["depth_mm"] = float(dims.group(2))
        if dims.group(3) is not None:
            values["thickness_mm"] = float(dims.group(3))

    width = _first_float(rf"(?:width|wide)\s*(?:=|:|of)?\s*{NUMBER}", text)
    depth = _first_float(rf"(?:depth|length|long)\s*(?:=|:|of)?\s*{NUMBER}", text)
    thickness = _first_float(rf"(?:thickness|thick)\s*(?:=|:|of)?\s*{NUMBER}", text)
    hole_dia = _first_float(
        rf"(?:hole|holes)\s*(?:diameter|dia|ø)?\s*(?:=|:|of)?\s*{NUMBER}",
        text,
    )
    inset = _first_float(rf"(?:inset|edge offset|margin)\s*(?:=|:|of)?\s*{NUMBER}", text)

    if width is not None:
        values["width_mm"] = width
    if depth is not None:
        values["depth_mm"] = depth
    if thickness is not None:
        values["thickness_mm"] = thickness
    if hole_dia is not None:
        values["hole_diameter_mm"] = hole_dia
    if inset is not None:
        values["hole_inset_mm"] = inset

    if re.search(r"\b(?:no holes|solid plate|without holes)\b", text):
        values["hole_count"] = 0
    elif re.search(r"\b(?:4|four)\s+(?:mounting\s+)?holes?\b", text):
        values["hole_count"] = 4

    result = PlateModel(**values)
    result.validate()
    return result


def openscad_source(model: PlateModel) -> str:
    model.validate()
    lines = [
        "// FormFactor deterministic CAD export",
        "// Units: millimeters",
        f"width = {model.width_mm:.4f};",
        f"depth = {model.depth_mm:.4f};",
        f"thickness = {model.thickness_mm:.4f};",
        f"hole_d = {model.hole_diameter_mm:.4f};",
        f"hole_inset = {model.hole_inset_mm:.4f};",
        "",
        "difference() {",
        "  cube([width, depth, thickness], center=false);",
    ]
    for x, y in model.hole_centers():
        lines.append(
            f"  translate([{x:.4f}, {y:.4f}, -0.5000]) "
            "cylinder(h=thickness + 1.0000, d=hole_d, $fn=48);"
        )
    lines.extend(["}", ""])
    return "\n".join(lines)


def freecad_source(model: PlateModel, step_path: Path, fcstd_path: Path | None = None) -> str:
    model.validate()
    centers = ", ".join(f"({x:.4f}, {y:.4f})" for x, y in model.hole_centers())
    lines = [
        "# FormFactor deterministic FreeCAD export",
        "import FreeCAD as App",
        "import Part",
        "",
        f"width = {model.width_mm:.4f}",
        f"depth = {model.depth_mm:.4f}",
        f"thickness = {model.thickness_mm:.4f}",
        f"hole_d = {model.hole_diameter_mm:.4f}",
        f"hole_centers = [{centers}]",
        "",
        'doc = App.newDocument("FormFactorCAD")',
        "shape = Part.makeBox(width, depth, thickness)",
        "for x, y in hole_centers:",
        "    tool = Part.makeCylinder(hole_d / 2.0, thickness + 1.0, App.Vector(x, y, -0.5))",
        "    shape = shape.cut(tool)",
        "",
        'obj = doc.addObject("PartDesign::Feature", "FormFactorPlate")',
        'obj.Label = "FormFactor Plate"',
        "obj.Shape = shape",
        "doc.recompute()",
        f"Part.export([obj], {str(step_path)!r})",
    ]
    if fcstd_path:
        lines.append(f"doc.saveAs({str(fcstd_path)!r})")
    lines.append("")
    return "\n".join(lines)


def _resolve_executable(candidates: Iterable[str]) -> str | None:
    for candidate in candidates:
        path = shutil.which(candidate)
        if path:
            return path
    return None


def export_with_openscad(model: PlateModel, output_path: Path, source_path: Path | None = None) -> Path:
    executable = _resolve_executable(("openscad", "openscad.exe"))
    if not executable:
        raise RuntimeError("OpenSCAD was not found on PATH")
    source_path = source_path or output_path.with_suffix(".scad")
    source_path.parent.mkdir(parents=True, exist_ok=True)
    source_path.write_text(openscad_source(model), encoding="utf-8")
    output_path.parent.mkdir(parents=True, exist_ok=True)
    subprocess.run([executable, "-o", str(output_path), str(source_path)], check=True)
    if not output_path.exists() or output_path.stat().st_size == 0:
        raise RuntimeError("OpenSCAD completed without producing a non-empty output")
    return output_path


def export_with_freecad(model: PlateModel, output_path: Path, macro_path: Path | None = None) -> Path:
    executable = _resolve_executable(("FreeCADCmd", "freecadcmd", "FreeCADCmd.exe", "freecadcmd.exe"))
    if not executable:
        raise RuntimeError("FreeCADCmd was not found on PATH")
    macro_path = macro_path or output_path.with_suffix(".freecad.py")
    macro_path.parent.mkdir(parents=True, exist_ok=True)
    macro_path.write_text(freecad_source(model, output_path), encoding="utf-8")
    output_path.parent.mkdir(parents=True, exist_ok=True)
    subprocess.run([executable, str(macro_path)], check=True)
    if not output_path.exists() or output_path.stat().st_size == 0:
        raise RuntimeError("FreeCAD completed without producing a non-empty STEP output")
    return output_path


def write_manifest(model: PlateModel, output_path: Path, engine: str, artifact: Path) -> Path:
    manifest = {
        "schema": "formfactor-cad-artifact-v1",
        "engine": engine,
        "model": asdict(model),
        "artifact": str(artifact),
    }
    output_path.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return output_path


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="FormFactor parametric CAD bridge")
    parser.add_argument(
        "--prompt",
        default="100x70x2 mm plate, 4 holes diameter 3.2mm, inset 5mm",
        help="Plain-language plate description",
    )
    parser.add_argument("--engine", choices=("source", "openscad", "freecad"), default="source")
    parser.add_argument("--output", type=Path, default=Path("build/cad/formfactor_plate.scad"))
    parser.add_argument("--manifest", type=Path, default=None)
    return parser


def main(argv: list[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    try:
        model = parse_prompt(args.prompt)
        args.output.parent.mkdir(parents=True, exist_ok=True)

        if args.engine == "source":
            args.output.write_text(openscad_source(model), encoding="utf-8")
        elif args.engine == "openscad":
            export_with_openscad(model, args.output)
        else:
            export_with_freecad(model, args.output)

        if args.manifest:
            args.manifest.parent.mkdir(parents=True, exist_ok=True)
            write_manifest(model, args.manifest, args.engine, args.output)

        print(f"FormFactor CAD wrote {args.output}")
        return 0
    except (OSError, ValueError, RuntimeError, subprocess.CalledProcessError) as exc:
        print(f"FormFactor CAD error: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
