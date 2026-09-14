# Changelog

This file records published changes without rewriting shared Git history. Historical summaries preserve original commit hashes and distinguish verified behavior from information that still requires direct diff or runner evidence.

## 2026-09-14

### CR-003 — Deterministic SPICE execution evidence

FormFactor now validates and serializes caller-supplied SPICE execution evidence into an unambiguous, deterministic replay record containing the exact input deck, solver name and version, portable exit status, and raw standard-output and standard-error streams. Missing provenance, missing input, absent output evidence, multiline versions, and out-of-range exit codes fail closed; repeated identical evidence produces an identical record. This records execution evidence only and does not invoke ngspice, authenticate the supplied provenance, interpret solver output, or claim that a circuit is electrically correct.

### CR-002 — Strict warnings for the complete validation suite

The dependency-free validation script now compiles every implemented FormFactor core target with `-Werror` in addition to the existing C++20, Wall, Wextra, and Wpedantic checks. This makes compiler warnings a failure for component, catalogue, quantity, regression, circuit, digital, SPICE, stackup, layout, impedance, return-path, PDN, decoupling, and executable validation targets; it does not add a new engineering claim or invent an unimplemented gate. The authoritative acceptance criteria are that `python3 scripts/check_identity.py` passes, `sh scripts/validate.sh` completes all existing targets, and the existing workflow continues to run the suite; remote GitHub file updates were applied, but local compiler and sanitizer execution was unavailable in this connector run.

### CR-001 — Governance traceability ledger

The repository now has a durable changelog location for tracing published commits to implemented behavior, validation evidence, and limitations without changing existing commit hashes. This entry is documentation-only: it does not add an engineering gate, does not certify any unimplemented capability, and does not replace core validation. The current branch was inspected through the GitHub repository API; local compiler, test, sanitizer, and workflow execution were unavailable in this documentation-only connector run.

### Historical commit: a06e724

[Add token-free repository label management](https://github.com/1freetech/formfactor/commit/a06e724ede96e0fb311f10cf6d94af8fb614d3a7) added repository label-management support. The published commit message does not include enough validation detail to claim that label state or any engineering gate was verified here; the behavior remains documented as historical implementation evidence pending a direct diff and live label readback.

### Historical commit: b2da870

[Add formfactor-cr-1 regression fixture](https://github.com/1freetech/formfactor/commit/b2da87089fd203b4ff79db15b3a1aa37a91e6ea5) added a regression fixture associated with FormFactor change request 1. The published title identifies the fixture but does not establish its expected core output or test result, so this ledger records the fixture as historical evidence without claiming a new engineering capability passed.

### Historical commit: 10e098d

[Add canonical formfactor signal template](https://github.com/1freetech/formfactor/commit/10e098d69c59910b2dddf45e5e37c8f6b7e3eba7) added the canonical change-request signal template. This is a documentation and governance change; it does not authorize issue text to override the core and does not represent a physical, electrical, manufacturing, or simulation validation result.

### Historical commit: 9050ceb

[Add exact sourced catalogue properties](https://github.com/1freetech/formfactor/commit/9050ceb26612126bec2f72f12545ec5a3193a43b) added exact sourced catalogue-property behavior according to its published title. The title alone does not provide the complete acceptance evidence, so this ledger does not claim specific property values, sources, filters, or tests beyond the existence of the published commit.

## Accuracy and traceability rules

Unknown values remain unknown. A requirement-record-only capability is not physical evaluation. Provenance records the stated source relationship but is not proof that a numeric value is correct. The renderer, game layer, and advisory prompt cannot decide engineering truth. A design is called manufacturable, safe, or compliant only when every currently required implemented core gate explicitly passes and the core reports that result.
