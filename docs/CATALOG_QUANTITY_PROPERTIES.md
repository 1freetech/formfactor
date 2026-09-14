# Sourced exact catalogue quantity properties

Implemented scope: truth-layer catalogue data and exact numeric filtering. Advanced-PCB connection: exact, sourced voltage, current, resistance, capacitance, inductance, frequency, geometry, time, and power statements are prerequisites for honest component filtering and later solver inputs. This slice records, validates, and filters those statements; it does not implement a physical solver.

## 1. What is implemented

1. A catalogue entry may contain zero or more `CatalogQuantityProperty` records.
2. Each record has a stable machine ID, readable name, exact SI `Quantity`, value qualifier, optional source condition, and revisioned source metadata.
3. Supported qualifiers are `nominal`, `minimum`, `typical`, and `maximum`. They preserve the source's claim; formfactor does not turn a typical value into a guaranteed limit.
4. An absent condition is encoded separately from a present condition. Condition text is preserved as opaque source wording and is not evaluated or discarded.
5. A valid entry produces the versioned project-internal record `formfactor-catalog-quantity-properties-v1`. Properties are sorted by ID and text is byte-length-prefixed for deterministic replay.
6. An entry with no properties exports `property-count=0`. No value is inferred from its component family, legacy scalar ratings, symbol, footprint, pin count, or appearance.
7. Any catalogue, mapping, component, or quantity-property validation error suppresses the complete canonical property record.
8. An exact numeric filter selects a property ID and qualifier, then applies `at-least`, `at-most`, or `equal` using the exact SI comparison layer.
9. Missing properties, qualifier mismatches, and source-condition mismatches return `unknown`, never `match`. Conditioned values are eligible only when the filter supplies the exact condition text.
10. Invalid catalogue provenance, malformed filters, unsupported enumerations, and dimension mismatches fail closed without a decision record. Valid decisions produce the deterministic `formfactor-catalog-filter-v1` record.

The numerical layer follows the [BIPM SI Brochure](https://www.bipm.org/en/publications/si-brochure) and [NIST SP 811](https://www.nist.gov/pml/owm/si-units-information). The catalogue record layout and identifier grammar are original formfactor formats, not external standards.

## 2. Validation contract

1. A property ID starts with a lowercase ASCII letter. It may then use lowercase letters, digits, `.`, `_`, or `-`, and must end with a letter or digit.
2. Property IDs are unique within one catalogue entry.
3. Display names, present conditions, and all source fields must contain visible, single-line text. Control characters are rejected so they cannot forge fields in an exported record.
4. Unknown qualifier identifiers fail closed.
5. The `Quantity` type can only be created through its validated exact-decimal factory. Equivalent inputs such as `1 V` and `1000 mV` therefore produce the same SI value and the same catalogue record.
6. Source metadata requires a title, URL, and revision. This is structural provenance: formfactor does not authenticate the URL, classify the publisher, or verify that the recorded number was copied correctly.

## 3. Measured acceptance results

The catalogue tests cover:

1. valid multi-property records and all four supported qualifiers;
2. identical records after repeated validation and reversed input order;
3. identical records for equivalent volt and millivolt inputs;
4. explicit zero-property and condition-present/condition-absent states;
5. the maximum supported signed coefficient and decimal exponent without floating-point conversion;
6. rejection of duplicate or malformed IDs, unsupported qualifiers, empty/control-bearing labels and conditions, and incomplete/control-bearing source metadata;
7. rejection of a catalogue ID containing a forged record line;
8. suppression of canonical output for every invalid case, including invalid legacy component ratings.

The filter tests additionally cover equivalent units, exact inclusive boundaries, definite non-matches, missing values, qualifier and condition mismatches, incompatible dimensions, invalid enumerations and IDs, deterministic records, and export blocking when source provenance is incomplete. All part names, URLs, assets, and numbers in the tests are synthetic fixtures. They are not verified manufacturer components.

## 4. Limits kept visible

- Property IDs do not yet belong to a typed semantic schema. This slice preserves the exact dimension carried by each `Quantity`, but it does not infer that a free-form ID is electrically meaningful or appropriate for a component family.
- Qualifier relationships, tolerances, sign/range plausibility, operating ranges, and cross-checks against legacy `double` rating fields are not implemented.
- Condition text is not machine-interpreted. A future filter must treat a conditioned value conservatively rather than silently ignoring its conditions.
- A source record is not proof of authenticity or measurement accuracy.
- No multi-constraint catalogue search, typed semantic property schema, graphical card, schematic symbol renderer, safety check, solver, or physical compliance result is added here. The filter evaluates only exact stored claims and does not convert a source claim into manufacturer verification.
