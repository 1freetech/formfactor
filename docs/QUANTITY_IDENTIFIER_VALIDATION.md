# Exact quantity identifier validation

Implemented scope: truth-layer unit identity. Advanced-PCB connection: reliable dimensions are prerequisites for catalogue property filters and for voltage, current, impedance, timing, geometry, power, and thermal solver inputs. No catalogue filter or physical solver is added here.

## 1. What changed

1. `make_quantity` now rejects every `Unit` identifier not explicitly listed by pcbtech.
2. `make_si_quantity` now rejects every `Dimension` identifier not explicitly listed by pcbtech.
3. Unknown units can no longer fall through to a dimensionless quantity.
4. Unknown dimensions can no longer produce a canonical record labelled `invalid`.
5. Both factories return an empty `std::optional` at the import boundary. The caller must preserve the value as unknown or report an error; pcbtech does not guess the intended unit.

The regression test first demonstrated that out-of-range enum values were accepted by parent commit `59d954ce96b3ccf7b641a891abce83499fb63948`. The previous unit switch returned the dimensionless definition when it did not recognize a value, while the SI factory accepted an unrecognized dimension unchanged.

## 2. Authority and project boundary

The supported dimensions, units, and decimal-prefix scaling remain based on the [BIPM SI Brochure, ninth edition](https://www.bipm.org/en/publications/si-brochure) and [NIST Special Publication 811](https://www.nist.gov/pml/owm/owm-products-and-services/publications-and-documentary-standards/metric-publications). The BIPM page identifies the ninth edition and its current update. pcbtech's enum identifiers and factory rejection behavior are an original internal API boundary, not an SI file format or a claim that all SI quantities are implemented.

## 3. Tested acceptance cases

1. All 33 declared `Unit` values construct successfully and map to the expected one of ten supported physical dimensions.
2. All ten declared `Dimension` values construct successfully through the SI factory.
3. Equivalent prefix inputs still compare exactly, including `1 V == 1000 mV`, `1 uF == 1000 nF == 1000000 pF`, and `2.2 kohm == 2200 ohm`.
4. Four unsupported unit identifiers and four unsupported dimension identifiers are rejected across three coefficient boundaries and three exponent boundaries: 72 unique invalid combinations.
5. Every invalid combination is evaluated twice and gives the same rejection result. No rejected input can reach canonical-record generation through a successful factory result.
6. Existing coefficient, normalization, ordering, dimension-mismatch, zero, and exponent-range cases remain covered.

The invalid identifiers include the value immediately before the first enum, the value immediately after the last enum, a distant value, and the largest signed `int`. Coefficients include `INT64_MIN`, zero, and `INT64_MAX`; exponents include -30, 0, and +30.

## 4. Limits that remain visible

- The exact quantity layer supports only the enumerated dimensions and units. Unsupported units remain unavailable rather than being inferred.
- Temperature is still excluded because Celsius conversion is affine, not a simple decimal-prefix scale.
- The supported exponent range remains -30 through +30. Values outside it are rejected rather than rounded or clamped.
- A valid unit identity says nothing about whether a numeric value is physically plausible, sourced, within a component rating, or safe for fabrication.
- Catalogue quantity properties, filters, solver records, graphical cards, and conversion from legacy `double` component ratings remain pending.

## 5. Run the test

Prerequisite: open a terminal in the pcbtech repository with GCC installed.

1. Run the complete engineering suite:

   ```bash
   CXX='g++ -Werror' sh scripts/validate.sh
   ```

2. Wait for the final line:

   ```text
   pcbtech component gate: PASS
   ```

The command tests the engineering core. It does not certify a board or validate the SDL workbench graphics.
