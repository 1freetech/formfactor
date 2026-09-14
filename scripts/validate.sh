#!/usr/bin/env sh
set -eu

mkdir -p build
cxx="${CXX:-g++}"
flags="-std=c++20 -Wall -Wextra -Wpedantic -Werror -Iinclude"

sources="src/component.cpp src/catalog.cpp src/catalog_filter.cpp src/quantity.cpp src/circuit.cpp src/digital.cpp src/spice.cpp src/stackup.cpp src/layout.cpp src/impedance.cpp src/return_path.cpp src/pdn.cpp src/decoupling.cpp"

$cxx $flags $sources tests/component_tests.cpp -o build/component_tests
./build/component_tests
$cxx $flags $sources tests/catalog_tests.cpp -o build/catalog_tests
./build/catalog_tests
$cxx $flags $sources tests/catalog_filter_tests.cpp -o build/catalog_filter_tests
./build/catalog_filter_tests
$cxx $flags $sources tests/quantity_tests.cpp -o build/quantity_tests
./build/quantity_tests
$cxx $flags $sources \
  tests/regression/formfactor_cr_1_invalid_unit_identifiers.cpp \
  -o build/regression_formfactor_cr_1
./build/regression_formfactor_cr_1
$cxx $flags $sources \
  tests/regression/formfactor_cr_5_catalog_property_schema.cpp \
  -o build/regression_formfactor_cr_5
./build/regression_formfactor_cr_5
$cxx $flags $sources \
  tests/regression/formfactor_cr_6_catalog_schema_coverage.cpp \
  -o build/regression_formfactor_cr_6
./build/regression_formfactor_cr_6
$cxx $flags $sources \
  tests/regression/formfactor_cr_7_component_family_applicability.cpp \
  -o build/regression_formfactor_cr_7
./build/regression_formfactor_cr_7
$cxx $flags $sources \
  tests/regression/formfactor_cr_8_property_component_identity.cpp \
  -o build/regression_formfactor_cr_8
./build/regression_formfactor_cr_8
$cxx $flags $sources tests/circuit_tests.cpp -o build/circuit_tests
./build/circuit_tests
$cxx $flags $sources tests/digital_tests.cpp -o build/digital_tests
./build/digital_tests
$cxx $flags $sources tests/spice_tests.cpp -o build/spice_tests
./build/spice_tests
$cxx $flags $sources tests/stackup_tests.cpp -o build/stackup_tests
./build/stackup_tests
$cxx $flags $sources tests/layout_tests.cpp -o build/layout_tests
./build/layout_tests
$cxx $flags $sources tests/impedance_tests.cpp -o build/impedance_tests
./build/impedance_tests
$cxx $flags $sources tests/return_path_tests.cpp -o build/return_path_tests
./build/return_path_tests
$cxx $flags $sources tests/pdn_tests.cpp -o build/pdn_tests
./build/pdn_tests
$cxx $flags $sources tests/decoupling_tests.cpp -o build/decoupling_tests
./build/decoupling_tests
$cxx $flags $sources src/main.cpp -o build/formfactor_validate
./build/formfactor_validate
