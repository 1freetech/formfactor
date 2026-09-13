# Component rating numeric gate

Implemented scope: truth layer, component numeric inputs. Advanced-PCB connection: trustworthy voltage, current, and temperature inputs are prerequisites for later power, thermal, and high-voltage checks. No such physical safety analysis is added here.

## 1. What changed

1. Reject NaN, which means "not a number", and positive or negative infinity in the three legacy component ratings.
2. Keep rejecting zero and negative ratings under the existing component-record contract.
3. Name the failing field and its unit: volts (`V`), amperes (`A`), or degrees Celsius (`deg C`).
4. Block the component's export gate and propagate the error into catalogue validation. Linking symbol, footprint, and model records cannot conceal an invalid rating.

Before the fix, checking only `x <= 0` missed NaN and positive infinity. An otherwise complete synthetic component could therefore pass the export gate with these values. The new regression test reproduced that failure before the implementation changed.

## 2. Numerical rule and references

Each of `max_voltage_v`, `max_current_a`, and `max_junction_c` must satisfy:

```text
std::isfinite(x) && x > 0
```

The standard classification function distinguishes finite floating-point values from infinity and NaN. It does not estimate a missing rating, replace the input, convert its unit, or validate a datasheet. See the [C++ working draft's classification functions](https://eel.is/c++draft/c.math.fpclass) and [WG14 N1570, section 7.12.3.2](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf).

Use ordinary standards-conforming floating-point compilation. `-ffinite-math-only`, `-ffast-math`, and `-Ofast` are unsupported for these checks because they permit the compiler to assume NaN/infinity cannot occur. No build-time flag guard is implemented in this slice. See [GCC's optimization options](https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html).

## 3. Tested acceptance cases

1. All three accuracy tiers reject each invalid rating independently: NaN, both infinities, positive and negative zero, a negative normal value, and a negative subnormal value. A subnormal is a very small representable floating-point number. These are 63 field/value/tier combinations.
2. Positive finite inputs retain the previous numeric acceptance behavior, including small and large representation boundaries. These are 54 field/value/tier combinations. Only a requested verified tier can pass the existing component export gate.
3. Repeating validation produces the same errors, error order, tier, and export decision. Three simultaneous invalid fields produce three ordered, unit-labelled errors.
4. Supplied scalar values stay unchanged; there is no implicit voltage, current, or temperature conversion.
5. The catalogue rejects all 27 non-finite field/value/tier combinations and preserves the component error with its `component:` prefix. Repeating the check produces the same errors.
6. Existing missing-source, missing-model, default-record, and zero-rating rejection remains intact.

The fixtures in `tests/component_tests.cpp` and `tests/catalog_tests.cpp` are synthetic. Their example sources and model flags are not evidence for a real manufacturer part.

## 4. Limits that remain visible

- Finite and positive does not mean physically plausible or manufacturer-verified. The largest finite test value is a numerical boundary, not an engineering recommendation.
- Requiring a positive Celsius maximum preserves the legacy API rule; it is not a universal law about valid component temperatures. Temperature ranges and exact affine Celsius/Kelvin conversions require separate work.
- The legacy fields are still binary floating-point scalars. The exact decimal quantity API is not yet integrated into component or catalogue properties.
- Source/model presence checks do not authenticate a document, inspect a solver model, or prove a pinout. Passing this gate alone does not establish electrical, thermal, creepage/clearance, or fabrication compliance.
- No new catalogue search, graphical component card, solver, or certificate-delivery feature is included.

## 5. Run the available engineering tests

Prerequisites: the repository source, Ubuntu/WSL, and GCC with C++20 support, as described in [Getting started](GETTING_STARTED.md). CMake is not needed for this path.

1. Open a terminal in the repository folder containing `scripts`.
2. Run:

   ```bash
   CXX='g++ -Werror' sh scripts/validate.sh
   ```

3. Wait for `pcbtech component gate: PASS`. The script stops if compilation or any test fails. `-Werror` treats compiler warnings as errors.

This command tests the engineering core; it does not launch or validate the SDL workbench.

## 6. Verification record, 2026-09-13

1. Reproduced a failing export-safety assertion against parent `c66ef9f600d4329a075491ae04de750ebf2b06e5` before fixing the component validator.
2. Passed the focused component and catalogue tests with GCC 13.3.0, C++20, `-O2`, and `-Wall -Wextra -Wpedantic -Werror` on x86_64 Ubuntu.
3. Passed all 12 engineering test executables and the component demonstration through the fallback script with `-Werror`.
4. Passed all 12 test executables and the demonstration with AddressSanitizer and UndefinedBehaviorSanitizer, strict warnings, and `-O1`. Leak scanning was disabled only after LeakSanitizer reported that it could not inspect `/proc/.../task`. Leak detection is unverified in this environment.
5. Rebuilt an empty intermediate `layout.o` encountered during the sanitizer build, then reran the complete sanitized suite successfully. No layout source change was required.
6. CMake and pkg-config were unavailable here. No CMake/SDL build or GUI validation is claimed. The dependency-free engineering suite remained available and passed.

<details>
<summary>Developer details: reproduce the sanitized core suite</summary>

Run in the repository terminal with GCC installed. Keep assertions enabled; do not add `-DNDEBUG` or fast-math flags. The non-PIE flags below describe the tested Linux configuration. `detect_leaks=0` retains address and undefined-behavior checks but omits leak scanning.

```sh
set -eu
mkdir -p build/rating-sanitize
flags='-std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all -fno-pie -no-pie -Iinclude'
sources='src/component.cpp src/catalog.cpp src/quantity.cpp src/circuit.cpp src/digital.cpp src/spice.cpp src/stackup.cpp src/layout.cpp src/impedance.cpp src/return_path.cpp src/pdn.cpp src/decoupling.cpp'
set --
for source in $sources; do
  name=$(basename "$source" .cpp)
  object="build/rating-sanitize/$name.o"
  g++ $flags -c "$source" -o "$object"
  test -s "$object"
  set -- "$@" "$object"
done
export ASAN_OPTIONS=detect_leaks=0:abort_on_error=1
export UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1
for test in tests/*_tests.cpp; do
  name=$(basename "$test" .cpp)
  g++ $flags "$test" "$@" -o "build/rating-sanitize/$name"
  "./build/rating-sanitize/$name"
  printf 'PASS %s (ASan + UBSan)\n' "$name"
done
g++ $flags src/main.cpp "$@" -o build/rating-sanitize/pcbtech_validate
./build/rating-sanitize/pcbtech_validate
```

</details>
