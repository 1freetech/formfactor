# pcbtech build sequence

Apply [simple instructions and component selection rules](INTERACTION_RULES.md) throughout this sequence. These rules are documented requirements; the component browser and graphical workbench are not implemented yet. The current executable is covered by [Run pcbtech in VS Code](GETTING_STARTED.md).

1. Component provenance and accuracy gates.
   - Pending: versioned catalogue records linking the selected part, symbol, footprint, physical model, and explicit pin mappings. Include functional categories and unit-aware property filters. Follow the acceptance cases in the interaction rules.
2. Typed pins, nets, and topology-only electrical-rule validation. (initial slice complete)
3. Deterministic three-state digital event simulator. (initial slice complete)
4. SPICE adapter with reproducible reference fixtures. (canonical passive/DC operating-point export slice complete; solver execution pending)
5. Board stack-up, geometry, clearance, and current constraints. (stackup, exact geometry, circular and trace-to-circular clearance, and sourced per-trace current-limit slices complete; trace-to-trace/shape clearance and calculated ampacity pending)
   - Controlled impedance: deterministic sourced single-ended/differential constraint representation complete; solver evaluation pending.
   - Return paths: deterministic trace-to-reference-net requirement representation complete; plane geometry evaluation pending.
   - Power integrity: exact rational target-impedance requirement model complete; PDN/decoupling solver evaluation pending.
   - Decoupling: sourced capacitor electrical and placement requirement model complete; component-model and PDN-response evaluation pending.
6. Thermal and power solvers with uncertainty reporting.
7. KiCad import/export and DRC comparison.
8. Interactive 3D board-construction interface.
   - Pending: labelled category buttons, searchable component cards with symbol/package previews, linked selection across views, and numbered guidance showing one action and its expected result. Keep engineering details available through an explicit details control.
9. Manufacturability gates and reproducible export bundles.

Each development cycle must make one bounded improvement, add or strengthen a test, run the full validation suite, and save only passing work.
