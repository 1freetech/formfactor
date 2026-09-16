#!/usr/bin/env python3
from __future__ import annotations

import importlib.util
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BRIDGE = ROOT / "tools" / "cad" / "formfactor_cad.py"

spec = importlib.util.spec_from_file_location("formfactor_cad_bridge", BRIDGE)
if spec is None or spec.loader is None:
    raise RuntimeError("could not load FormFactor CAD bridge")
cad = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = cad
spec.loader.exec_module(cad)


class CadBridgeTests(unittest.TestCase):
    def test_prompt_updates_dimensions_and_holes(self) -> None:
        model = cad.parse_prompt(
            "120x80x2 mm plate, 4 mounting holes diameter 3.2mm, inset 5mm"
        )
        self.assertEqual(model.width_mm, 120.0)
        self.assertEqual(model.depth_mm, 80.0)
        self.assertEqual(model.thickness_mm, 2.0)
        self.assertEqual(model.hole_diameter_mm, 3.2)
        self.assertEqual(model.hole_inset_mm, 5.0)
        self.assertEqual(
            model.hole_centers(),
            ((5.0, 5.0), (115.0, 5.0), (115.0, 75.0), (5.0, 75.0)),
        )

    def test_solid_plate_removes_holes(self) -> None:
        model = cad.parse_prompt("80x50x1.6 mm solid plate without holes")
        self.assertEqual(model.hole_count, 0)
        self.assertEqual(model.hole_centers(), ())

    def test_openscad_output_is_deterministic(self) -> None:
        model = cad.PlateModel()
        first = cad.openscad_source(model)
        second = cad.openscad_source(model)
        self.assertEqual(first, second)
        self.assertIn("difference() {", first)
        self.assertEqual(first.count("cylinder("), 4)
        self.assertIn("cube([width, depth, thickness]", first)

    def test_freecad_source_has_real_boolean_and_step_export(self) -> None:
        model = cad.PlateModel(width_mm=90.0, depth_mm=60.0)
        source = cad.freecad_source(model, Path("plate.step"))
        self.assertIn("Part.makeBox", source)
        self.assertIn("Part.makeCylinder", source)
        self.assertIn("shape.cut(tool)", source)
        self.assertIn("Part.export([obj], 'plate.step')", source)

    def test_invalid_hole_pattern_fails_closed(self) -> None:
        with self.assertRaises(ValueError):
            cad.PlateModel(
                width_mm=20.0,
                depth_mm=20.0,
                hole_diameter_mm=8.0,
                hole_inset_mm=3.0,
            ).validate()

    def test_source_cli_writes_non_empty_scad_and_manifest(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            output = root / "model.scad"
            manifest = root / "model.json"
            code = cad.main(
                [
                    "--prompt",
                    "100x70x2 mm plate, four holes diameter 3.2, inset 5",
                    "--engine",
                    "source",
                    "--output",
                    str(output),
                    "--manifest",
                    str(manifest),
                ]
            )
            self.assertEqual(code, 0)
            self.assertTrue(output.exists())
            self.assertGreater(output.stat().st_size, 0)
            self.assertTrue(manifest.exists())
            self.assertIn("formfactor-cad-artifact-v1", manifest.read_text(encoding="utf-8"))


if __name__ == "__main__":
    unittest.main()
