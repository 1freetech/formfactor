cmake_minimum_required(VERSION 3.20)
project(formfactor VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(FORMFACTOR_BUILD_FREELAB "Build the SDL2 graphical workbench when dependencies are available" ON)
option(FORMFACTOR_WARNINGS_AS_ERRORS "Treat FormFactor compiler warnings as errors" ON)
option(FORMFACTOR_BUILD_SIGNATURE_VERIFIER "Build the OpenSSL-backed Ed25519 policy signature verifier" ON)
option(FORMFACTOR_ENABLE_BOX3D_PHYSICS "Build the optional Box3D 3D physics backend" OFF)

if(FORMFACTOR_ENABLE_BOX3D_PHYSICS)
  if(CMAKE_VERSION VERSION_LESS 3.22)
    message(FATAL_ERROR "Box3D physics requires CMake 3.22 or newer")
  endif()
  enable_language(C)
  include(FetchContent)
  FetchContent_Declare(
    box3d
    GIT_REPOSITORY https://github.com/erincatto/box3d.git
    GIT_TAG v0.1.0
    GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(box3d)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
  if(FORMFACTOR_WARNINGS_AS_ERRORS)
    add_compile_options(-Werror)
  endif()
endif()

add_library(formfactor_core src/component.cpp src/component_geometry.cpp src/hash.cpp src/policy_trust.cpp src/key_authority.cpp src/source_authority.cpp src/catalog.cpp src/catalog_filter.cpp src/quantity.cpp src/circuit.cpp src/digital.cpp src/spice.cpp src/stackup.cpp src/layout.cpp src/impedance.cpp src/return_path.cpp src/pdn.cpp src/decoupling.cpp)
target_include_directories(formfactor_core PUBLIC include)

if(FORMFACTOR_ENABLE_BOX3D_PHYSICS)
  add_library(formfactor_box3d_physics src/box3d_physics.cpp)
  target_include_directories(formfactor_box3d_physics PUBLIC include)
  target_link_libraries(formfactor_box3d_physics PUBLIC box3d::box3d)
endif()

if(FORMFACTOR_BUILD_SIGNATURE_VERIFIER)
  find_package(OpenSSL 1.1.1 REQUIRED COMPONENTS Crypto)
  add_library(formfactor_policy_signature src/policy_signature.cpp)
  target_include_directories(formfactor_policy_signature PUBLIC include)
  target_link_libraries(formfactor_policy_signature PUBLIC formfactor_core OpenSSL::Crypto)
endif()

add_executable(formfactor_validate src/main.cpp)
target_link_libraries(formfactor_validate PRIVATE formfactor_core)

if(FORMFACTOR_BUILD_FREELAB)
  find_package(PkgConfig QUIET)
  if(PkgConfig_FOUND)
    pkg_check_modules(SDL2 QUIET IMPORTED_TARGET sdl2)
  endif()

  if(TARGET PkgConfig::SDL2)
    add_executable(formfactor_freelab src/workbench_plus.cpp)
    target_link_libraries(formfactor_freelab PRIVATE formfactor_core PkgConfig::SDL2)
    if(FORMFACTOR_ENABLE_BOX3D_PHYSICS)
      target_link_libraries(formfactor_freelab PRIVATE formfactor_box3d_physics)
      target_compile_definitions(formfactor_freelab PRIVATE FORMFACTOR_HAS_BOX3D=1)
    endif()
  else()
    message(STATUS "FreeLab disabled: pkg-config and SDL2 development files were not found")
  endif()
endif()

enable_testing()
add_executable(component_tests tests/component_tests.cpp)
target_link_libraries(component_tests PRIVATE formfactor_core)
add_test(NAME component_validation COMMAND component_tests)

add_executable(component_geometry_tests tests/component_geometry_tests.cpp)
target_link_libraries(component_geometry_tests PRIVATE formfactor_core)
add_test(NAME component_geometry_catalog COMMAND component_geometry_tests)

add_executable(catalog_tests tests/catalog_tests.cpp)
target_link_libraries(catalog_tests PRIVATE formfactor_core)
add_test(NAME catalog_linkage COMMAND catalog_tests)

add_executable(catalog_filter_tests tests/catalog_filter_tests.cpp)
target_link_libraries(catalog_filter_tests PRIVATE formfactor_core)
add_test(NAME catalog_exact_filter COMMAND catalog_filter_tests)

add_executable(quantity_tests tests/quantity_tests.cpp)
target_link_libraries(quantity_tests PRIVATE formfactor_core)
add_test(NAME exact_quantity_units COMMAND quantity_tests)

add_executable(
  regression_formfactor_cr_1
  tests/regression/formfactor_cr_1_invalid_unit_identifiers.cpp)
target_link_libraries(regression_formfactor_cr_1 PRIVATE formfactor_core)
add_test(
  NAME regression_formfactor_cr_1_invalid_unit_identifiers
  COMMAND regression_formfactor_cr_1)

add_executable(
  regression_formfactor_cr_5
  tests/regression/formfactor_cr_5_catalog_property_schema.cpp)
target_link_libraries(regression_formfactor_cr_5 PRIVATE formfactor_core)
add_test(
  NAME regression_formfactor_cr_5_catalog_property_schema
  COMMAND regression_formfactor_cr_5)

add_executable(
  regression_formfactor_cr_6
  tests/regression/formfactor_cr_6_catalog_schema_coverage.cpp)
target_link_libraries(regression_formfactor_cr_6 PRIVATE formfactor_core)
add_test(
  NAME regression_formfactor_cr_6_catalog_schema_coverage
  COMMAND regression_formfactor_cr_6)

add_executable(
  regression_formfactor_cr_7
  tests/regression/formfactor_cr_7_component_family_applicability.cpp)
target_link_libraries(regression_formfactor_cr_7 PRIVATE formfactor_core)
add_test(
  NAME regression_formfactor_cr_7_component_family_applicability
  COMMAND regression_formfactor_cr_7)

add_executable(
  regression_formfactor_cr_8
  tests/regression/formfactor_cr_8_property_component_identity.cpp)
target_link_libraries(regression_formfactor_cr_8 PRIVATE formfactor_core)
add_test(
  NAME regression_formfactor_cr_8_property_component_identity
  COMMAND regression_formfactor_cr_8)

add_executable(
  regression_formfactor_cr_9
  tests/regression/formfactor_cr_9_source_artifact_digest.cpp)
target_link_libraries(regression_formfactor_cr_9 PRIVATE formfactor_core)
add_test(
  NAME regression_formfactor_cr_9_source_artifact_digest
  COMMAND regression_formfactor_cr_9)

add_executable(
  regression_formfactor_cr_11
  tests/regression/formfactor_cr_11_source_locator.cpp)
target_link_libraries(regression_formfactor_cr_11 PRIVATE formfactor_core)
add_test(
  NAME regression_formfactor_cr_11_source_locator
  COMMAND regression_formfactor_cr_11)

add_executable(
  regression_formfactor_cr_12
  tests/regression/formfactor_cr_12_source_claim_text.cpp)
target_link_libraries(regression_formfactor_cr_12 PRIVATE formfactor_core)
add_test(
  NAME regression_formfactor_cr_12_source_claim_text
  COMMAND regression_formfactor_cr_12)

add_executable(
  regression_formfactor_cr_13
  tests/regression/formfactor_cr_13_https_source_urls.cpp)
target_link_libraries(regression_formfactor_cr_13 PRIVATE formfactor_core)
add_test(
  NAME regression_formfactor_cr_13_https_source_urls
  COMMAND regression_formfactor_cr_13)

add_executable(
  regression_formfactor_cr_14
  tests/regression/formfactor_cr_14_publisher_origin_authorization.cpp)
target_link_libraries(regression_formfactor_cr_14 PRIVATE formfactor_core)
add_test(
  NAME regression_formfactor_cr_14_publisher_origin_authorization
  COMMAND regression_formfactor_cr_14)

add_executable(
  regression_formfactor_cr_15
  tests/regression/formfactor_cr_15_authorization_policy_provenance.cpp)
target_link_libraries(regression_formfactor_cr_15 PRIVATE formfactor_core)
add_test(
  NAME regression_formfactor_cr_15_authorization_policy_provenance
  COMMAND regression_formfactor_cr_15)

add_executable(
  regression_formfactor_cr_16
  tests/regression/formfactor_cr_16_policy_artifact_digest_verification.cpp)
target_link_libraries(regression_formfactor_cr_16 PRIVATE formfactor_core)
add_test(
  NAME regression_formfactor_cr_16_policy_artifact_digest_verification
  COMMAND regression_formfactor_cr_16)

add_executable(
  regression_formfactor_cr_17
  tests/regression/formfactor_cr_17_ed25519_trust_anchor.cpp)
target_link_libraries(regression_formfactor_cr_17 PRIVATE formfactor_core)
add_test(
  NAME regression_formfactor_cr_17_ed25519_trust_anchor
  COMMAND regression_formfactor_cr_17)

add_executable(
  regression_formfactor_cr_18
  tests/regression/formfactor_cr_18_ed25519_signature_envelope.cpp)
target_link_libraries(regression_formfactor_cr_18 PRIVATE formfactor_core)
add_test(
  NAME regression_formfactor_cr_18_ed25519_signature_envelope
  COMMAND regression_formfactor_cr_18)

add_executable(
  regression_formfactor_cr_20
  tests/regression/formfactor_cr_20_pinned_key_trust_root.cpp)
target_link_libraries(regression_formfactor_cr_20 PRIVATE formfactor_core)
add_test(
  NAME regression_formfactor_cr_20_pinned_key_trust_root
  COMMAND regression_formfactor_cr_20)

add_executable(
  regression_formfactor_cr_21
  tests/regression/formfactor_cr_21_root_update_sequence.cpp)
target_link_libraries(regression_formfactor_cr_21 PRIVATE formfactor_core)
add_test(
  NAME regression_formfactor_cr_21_root_update_sequence
  COMMAND regression_formfactor_cr_21)

if(FORMFACTOR_BUILD_SIGNATURE_VERIFIER)
  add_executable(
    regression_formfactor_cr_19
    tests/regression/formfactor_cr_19_ed25519_signature_verification.cpp)
  target_link_libraries(
    regression_formfactor_cr_19 PRIVATE formfactor_policy_signature)
  add_test(
    NAME regression_formfactor_cr_19_ed25519_signature_verification
    COMMAND regression_formfactor_cr_19)
endif()

add_executable(circuit_tests tests/circuit_tests.cpp)
target_link_libraries(circuit_tests PRIVATE formfactor_core)
add_test(NAME circuit_validation COMMAND circuit_tests)

add_executable(digital_tests tests/digital_tests.cpp)
target_link_libraries(digital_tests PRIVATE formfactor_core)
add_test(NAME digital_simulation COMMAND digital_tests)

add_executable(spice_tests tests/spice_tests.cpp)
target_link_libraries(spice_tests PRIVATE formfactor_core)
add_test(NAME spice_export COMMAND spice_tests)

add_executable(stackup_tests tests/stackup_tests.cpp)
target_link_libraries(stackup_tests PRIVATE formfactor_core)
add_test(NAME stackup_validation COMMAND stackup_tests)

add_executable(layout_tests tests/layout_tests.cpp)
target_link_libraries(layout_tests PRIVATE formfactor_core)
add_test(NAME layout_validation COMMAND layout_tests)

add_executable(impedance_tests tests/impedance_tests.cpp)
target_link_libraries(impedance_tests PRIVATE formfactor_core)
add_test(NAME impedance_constraints COMMAND impedance_tests)

add_executable(return_path_tests tests/return_path_tests.cpp)
target_link_libraries(return_path_tests PRIVATE formfactor_core)
add_test(NAME return_path_constraints COMMAND return_path_tests)

add_executable(pdn_tests tests/pdn_tests.cpp)
target_link_libraries(pdn_tests PRIVATE formfactor_core)
add_test(NAME pdn_constraints COMMAND pdn_tests)

add_executable(decoupling_tests tests/decoupling_tests.cpp)
target_link_libraries(decoupling_tests PRIVATE formfactor_core)
add_test(NAME decoupling_requirements COMMAND decoupling_tests)

if(FORMFACTOR_ENABLE_BOX3D_PHYSICS)
  add_executable(box3d_physics_tests tests/box3d_physics_tests.cpp)
  target_link_libraries(box3d_physics_tests PRIVATE formfactor_box3d_physics)
  add_test(NAME box3d_physics_smoke COMMAND box3d_physics_tests)
endif()
