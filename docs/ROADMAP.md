# pcbtech build sequence

1. Component provenance and accuracy gates.
2. Typed pins, nets, and topology-only electrical-rule validation. (initial slice complete)
3. Deterministic three-state digital event simulator. (initial slice complete)
4. SPICE adapter with reproducible reference fixtures. (canonical passive/DC operating-point export slice complete; solver execution pending)
5. Board stack-up, geometry, clearance, and current constraints. (stackup plus exact-nanometre pad/via/trace and sourced-rule validation slices complete; spatial clearance pending)
6. Thermal and power solvers with uncertainty reporting.
7. KiCad import/export and DRC comparison.
8. Interactive 3D board-construction interface.
9. Manufacturability gates and reproducible export bundles.

Each development cycle must make one bounded improvement, add or strengthen a test, run the full validation suite, and save only passing work.
