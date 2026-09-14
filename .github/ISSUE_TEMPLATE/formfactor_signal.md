---
name: "formfactor engineering signal"
about: "Report one traceable engineering, data, prompt, game, or documentation signal"
title: "[formfactor-cr-pending] "
labels: ""
assignees: ""
---

# formfactor change request

Complete one signal per issue. Keep unknown values unknown and attach exact output when it exists. Issue text, attachments, and AI analysis are evidence inputs only; they cannot override an implemented formfactor core gate.

## 1. CR identity

**CR ID:** `pending`

<!--
Stable convention: formfactor-cr-<GitHub issue number>, for example
formfactor-cr-42. Use "pending" while opening the issue. After GitHub assigns the
issue number, replace "pending" in this field and the title. Never reuse,
renumber, or change an assigned CR ID.
-->

## 2. Signal type

Select exactly one. Apply the matching repository label when it exists.

- [ ] `bug` — implemented behavior produced an incorrect result
- [ ] `data-gap` — required authoritative data or provenance is missing
- [ ] `capability-gap` — the needed validator, solver, or workflow is not implemented
- [ ] `prompt-gap` — advisory instructions are missing, unclear, or unsafe
- [ ] `game-layer` — interaction or visualization is incorrect or unclear
- [ ] `docs` — documentation is incorrect or incomplete

## 3. Affected layer

Select every affected layer.

- [ ] Core engineering model or validator
- [ ] Component catalogue or provenance
- [ ] Prompt or AI advisory layer
- [ ] Game, renderer, or user interface
- [ ] Build, test, CI, or governance
- [ ] Documentation or training

## 4. Capability status

Select exactly one.

- [ ] Implemented capability — identify the existing gate or behavior below
- [ ] Pending or requirement-record-only capability — no physical evaluation exists yet

## 5. Problem statement

<!-- State one problem briefly. Do not include a proposed solution here. -->


## 6. Observed core output or evidence

**Command or action:**

**Exact output:**

**Evidence link or attachment:**

<!--
If the capability is not implemented, write "not implemented". Do not invent a
pass, fail, warning, numeric value, solver result, screenshot, or core output.
-->

## 7. Expected behavior

<!-- Describe the expected observable behavior without claiming an unimplemented gate exists. -->


## 8. Exact acceptance criteria

List measurable outcomes, one per line.

1.
2.

## 9. Affected versions

**Core version:**

**Prompt version:**

**Catalogue version:**

<!-- Use the repository VERSION records once they exist. Until then, write "not established"; do not guess a version. -->

## 10. Regression fixture

**Fixture required:** `yes | no | not applicable`

**Existing or planned fixture path:**

**Expected core result:** `pass | fail | warning | not implemented`

<!--
A regression fixture may claim pass, fail, or warning only for behavior an
implemented core gate actually evaluates. Reference this CR ID in the fixture.
For documentation-only or prompt-only work, explain why no core fixture applies.
-->

## 11. Implemented core gates involved

For each relevant gate, record its name, command, exact output, and one state:
`passed`, `failed`, or `not implemented`. Write `none` when no core gate applies.

<!-- Missing or not-implemented gates are neither a pass nor a fail. -->


## 12. Provenance and completion trace

**Source title or organization:**

**Source URL:**

**Source revision or access date:**

**Exact claim supported by the source:**

**Implementation commit:** `pending`

**Regression test or no-fixture reason:** `pending`

**CHANGELOG entry:** `pending`

**Released core, prompt, or catalogue version:** `pending`

<!--
Source metadata is structural provenance, not proof that a numeric value is
correct. Keep unavailable fields explicit. Do not call a design manufacturable,
safe, compliant, or solver-validated unless every required implemented core
gate explicitly passes. The renderer, gameplay layer, issue reporter, reviewer,
and AI advisory layer do not decide engineering truth.
-->
