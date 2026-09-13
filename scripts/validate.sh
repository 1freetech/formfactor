#!/usr/bin/env sh
set -eu

mkdir -p build
cxx="${CXX:-g++}"
flags="-std=c++20 -Wall -Wextra -Wpedantic -Iinclude"

$cxx $flags src/component.cpp tests/component_tests.cpp -o build/component_tests
./build/component_tests
$cxx $flags src/component.cpp src/main.cpp -o build/pcbtech_validate
./build/pcbtech_validate

