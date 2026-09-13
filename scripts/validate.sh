#!/usr/bin/env sh
set -eu

mkdir -p build
cxx="${CXX:-g++}"
flags="-std=c++20 -Wall -Wextra -Wpedantic -Iinclude"

sources="src/component.cpp src/circuit.cpp src/digital.cpp src/spice.cpp src/stackup.cpp src/layout.cpp"

$cxx $flags $sources tests/component_tests.cpp -o build/component_tests
./build/component_tests
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
$cxx $flags $sources src/main.cpp -o build/pcbtech_validate
./build/pcbtech_validate
