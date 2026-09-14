# Changelog

## CR-015 — Fail-closed authorization-policy provenance

Every publisher-origin authorization policy now requires visible revisioned HTTPS source metadata and a lowercase 64-hex SHA-256 digest binding the decision to the exact caller-supplied policy artifact. Missing, insecure, short, long, uppercase, and non-hex evidence fails closed as `Invalid` and suppresses the complete record; valid authorizations and denials advance to deterministic `formfactor-source-authorization-v2` records containing the policy source and digest. The core does not retrieve or hash the policy artifact, verify a signature, authenticate its publisher, or yet enforce the decision during catalogue export.

## CR-014 — Deterministic publisher-origin authorization

FormFactor now evaluates a source claim against an explicit caller-supplied publisher policy using exact visible publisher IDs and exact canonical HTTPS origins. Valid mismatches, including subdomains and explicit-port differences, produce `NotAuthorized`; malformed sources, publisher IDs, origins, empty policies, and duplicate origins fail closed as `Invalid` with no record. Valid decisions produce deterministic `formfactor-source-authorization-v1` length-prefixed records independent of policy-origin order. This primitive does not authenticate the caller policy, publisher ownership, DNS, TLS, network content, or artifact signatures, and it is not yet connected to catalogue export.

## CR-013 — Fail-closed canonical HTTPS source URLs

All source metadata now passes one shared conservative absolute-HTTPS URL validator before a verified component or catalogue claim can be exported. The validator follows an intentionally narrow RFC 3986 URI profile: lowercase `https://`, a lowercase ASCII DNS host with an optional numeric port, no credentials, no fragments, and no whitespace or control characters; malformed or unsupported URLs fail closed, deterministic validation is covered, and the one-character host is retained as the structural boundary. URL syntax does not authenticate a host, authorize a publisher, fetch an artifact, or prove any engineering claim.

## CR-012 — Fail-closed source-claim text evidence

Every exported catalogue quantity claim now preserves the exact visible, single-line wording copied from its SHA-256-bound source artifact. The claim text is length-prefixed in deterministic `formfactor-catalog-quantity-properties-v5` records; missing, whitespace-only, and control-bearing text fails closed and suppresses the complete export record. The gate preserves transcription audit evidence without parsing the prose, inferring a value, authenticating a publisher, or asserting that the structured quantity matches the source wording.

## CR-011 — Fail-closed source-claim locators

Every exported catalogue quantity claim now requires visible, single-line text identifying its exact location inside the SHA-256-bound source artifact, such as a published page, table, section, or stable anchor. The locator is length-prefixed in deterministic `formfactor-catalog-quantity-properties-v4` records; missing, whitespace-only, and control-bearing locators fail closed and suppress the complete export record. The field makes a claim auditable but remains opaque source wording: FormFactor does not infer a location, fetch the artifact, authenticate its publisher, or assert that the transcription is correct.

## CR-010 — Reproducible CMake and CTest validation

FormFactor can now configure and test its engineering core through CMake even when the optional FreeLab graphics dependencies are unavailable, because `pkg-config` and SDL2 no longer block core configuration and the workbench is skipped with an explicit status message when those packages are missing. A pinned workspace-local bootstrap command provides CMake and CTest without modifying system packages, compiler warnings are treated as errors consistently across CMake targets, and GitHub Actions now configures, builds, and runs the complete CTest suite in addition to the dependency-free validation path. The change was verified locally with CMake 4.4.3, GNU C++ 13.3.0, all 19 registered CTest cases, the identity check, and the complete fallback suite; FreeLab itself still requires the SDL2 development package and was not built in the dependency-limited runner.

This file records published changes without rewriting shared Git history. Historical summaries preserve original commit hashes and distinguish verified behavior from information that still requires direct diff or runner evidence.

## 2026-09-14

### CR-009 — Source-artifact integrity binding

Every exported catalogue quantity claim now requires a lowercase 64-hex SHA-256 digest of the exact source artifact used for transcription. The digest is included in deterministic `formfactor-catalog-quantity-properties-v3` records; missing, short, long, uppercase, and non-hex digests fail closed and suppress export. This integrity binding identifies source bytes but does not fetch the artifact, authenticate its publisher, establish document authority, or prove transcription correctness.

### CR-008 — Fail-closed property-to-component identity binding

FormFactor now requires every catalogue quantity claim to carry explicit manufacturer and part-number fields that exactly match the catalogue component before the claim can enter the deterministic export record, preventing a sourced value for one part from being silently attached to another part with a plausible label or family. The canonical record advances to `formfactor-catalog-quantity-properties-v2` so the added identity fields cannot be mistaken for the older layout; CR-008 covers valid binding, missing, mismatched, and control-bearing identity rejection, deterministic replay, and complete export suppression, while source URL authentication, transcription verification, publisher classification, physical plausibility, safety, compliance, and solver evaluation remain unimplemented.

### CR-007 — Fail-closed component-family applicability

FormFactor now attaches an explicit supported-family set to every implemented catalogue quantity schema and rejects a property when its entry's primary component family is not declared by that schema, so a plausible property name cannot silently cross family boundaries or enter an export record. The registry is a conservative project support contract rather than a claim that an excluded property is physically impossible; CR-007 covers valid family use, unsupported and `Other` rejection, deterministic repeat lookup, complete-record suppression, and existing dimension, qualifier, provenance, and unit gates. Source authentication, manufacturer-specific applicability, multiple-family entries, physical plausibility, safety, compliance, and solver evaluation remain unimplemented.

### CR-006 — Complete supported-dimension catalogue schema coverage

FormFactor now exposes an explicit catalogue-property schema for all four source qualifiers across capacitance, current, frequency, inductance, length, power, resistance, time, and voltage, covering every currently supported non-dimensionless exact quantity without deriving meaning from free-form text or inventing a component value. The new regression fixture checks all 36 identifier, dimension, and qualifier mappings, deterministic repeat lookup, and fail-closed rejection of unsupported temperature and plausible undeclared qualifier names; existing catalogue validation still suppresses export when a property's exact dimension or qualifier conflicts with its schema. Temperature remains unsupported until affine conversion is implemented, and component-family applicability, source authentication, physical plausibility, safety, compliance, and solver evaluation remain explicitly unimplemented.

### CR-005 — Typed catalogue-property schema gate

FormFactor now requires every exported catalogue quantity property to match an explicit implemented semantic schema that fixes both its physical dimension and value qualifier, preventing a free-form property name from silently assigning engineering meaning to an unrelated value. The initial registry covers nominal capacitance, maximum power, nominal resistance, and all four supported voltage qualifiers; unsupported identifiers, dimension mismatches, and qualifier conflicts fail closed and suppress the complete catalogue record. Tests cover valid lookup, deterministic lookup, unsupported semantics, incorrect units, incorrect qualifiers, and export blocking, while schema expansion, component-family applicability, provenance authentication, and physical safety or compliance decisions remain unimplemented.

### CR-004 — Fail-closed exact catalogue filtering

FormFactor can now evaluate one exact numeric catalogue constraint by property identity, value qualifier, comparison relation, and optional source condition while preserving three distinct outcomes: match, no match, and unknown. Exact SI comparison makes equivalent units and inclusive boundaries deterministic, while missing properties, mismatched qualifiers, and mismatched conditions remain unknown and can never satisfy a filter; incompatible dimensions, malformed requests, unsupported enumeration values, invalid catalogue records, and incomplete provenance fail closed without producing a decision record. The new tests cover valid, invalid, boundary, deterministic replay, equivalent-unit, provenance, and export-safety behavior, and the implementation does not claim that a sourced value is authentic, manufacturer-verified, safe, compliant, or physically solved.

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
