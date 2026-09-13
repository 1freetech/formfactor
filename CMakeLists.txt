cmake_minimum_required(VERSION 3.20)
project(pcbtech VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(pcbtech_core src/component.cpp src/circuit.cpp src/digital.cpp src/spice.cpp src/stackup.cpp src/layout.cpp src/impedance.cpp src/return_path.cpp src/pdn.cpp src/decoupling.cpp)
target_include_directories(pcbtech_core PUBLIC include)
target_compile_options(pcbtech_core PRIVATE -Wall -Wextra -Wpedantic)

add_executable(pcbtech_validate src/main.cpp)
target_link_libraries(pcbtech_validate PRIVATE pcbtech_core)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
add_executable(pcbtech_freelab src/workbench.cpp)
target_link_libraries(pcbtech_freelab PRIVATE pcbtech_core PkgConfig::SDL2)

enable_testing()
add_executable(component_tests tests/component_tests.cpp)
target_link_libraries(component_tests PRIVATE pcbtech_core)
add_test(NAME component_validation COMMAND component_tests)

add_executable(circuit_tests tests/circuit_tests.cpp)
target_link_libraries(circuit_tests PRIVATE pcbtech_core)
add_test(NAME circuit_validation COMMAND circuit_tests)

add_executable(digital_tests tests/digital_tests.cpp)
target_link_libraries(digital_tests PRIVATE pcbtech_core)
add_test(NAME digital_simulation COMMAND digital_tests)

add_executable(spice_tests tests/spice_tests.cpp)
target_link_libraries(spice_tests PRIVATE pcbtech_core)
add_test(NAME spice_export COMMAND spice_tests)

add_executable(stackup_tests tests/stackup_tests.cpp)
target_link_libraries(stackup_tests PRIVATE pcbtech_core)
add_test(NAME stackup_validation COMMAND stackup_tests)

add_executable(layout_tests tests/layout_tests.cpp)
target_link_libraries(layout_tests PRIVATE pcbtech_core)
add_test(NAME layout_validation COMMAND layout_tests)

add_executable(impedance_tests tests/impedance_tests.cpp)
target_link_libraries(impedance_tests PRIVATE pcbtech_core)
add_test(NAME impedance_constraints COMMAND impedance_tests)

add_executable(return_path_tests tests/return_path_tests.cpp)
target_link_libraries(return_path_tests PRIVATE pcbtech_core)
add_test(NAME return_path_constraints COMMAND return_path_tests)

add_executable(pdn_tests tests/pdn_tests.cpp)
target_link_libraries(pdn_tests PRIVATE pcbtech_core)
add_test(NAME pdn_constraints COMMAND pdn_tests)

add_executable(decoupling_tests tests/decoupling_tests.cpp)
target_link_libraries(decoupling_tests PRIVATE pcbtech_core)
add_test(NAME decoupling_requirements COMMAND decoupling_tests)
