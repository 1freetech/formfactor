cmake_minimum_required(VERSION 3.20)
project(pcbtech VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(pcbtech_core src/component.cpp)
target_include_directories(pcbtech_core PUBLIC include)
target_compile_options(pcbtech_core PRIVATE -Wall -Wextra -Wpedantic)

add_executable(pcbtech_validate src/main.cpp)
target_link_libraries(pcbtech_validate PRIVATE pcbtech_core)

enable_testing()
add_executable(component_tests tests/component_tests.cpp)
target_link_libraries(component_tests PRIVATE pcbtech_core)
add_test(NAME component_validation COMMAND component_tests)
