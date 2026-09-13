# pcbtech build sequence

1. Component provenance and accuracy gates.
2. Typed pins, nets, and topology-only electrical-rule validation. (initial slice complete)
3. Deterministic digital event simulator.
4. SPICE adapter with reproducible reference fixtures.
5. Board stack-up, geometry, clearance, and current constraints.
6. Thermal and power solvers with uncertainty reporting.
7. KiCad import/export and DRC comparison.
8. Interactive 3D board-construction interface.
9. Manufacturability gates and reproducible export bundles.

Each development cycle must make one bounded improvement, add or strengthen a test, run the full validation suite, and save only passing work.
