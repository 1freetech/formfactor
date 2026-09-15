cmake_minimum_required(VERSION 3.20)
project(FormFactorIOS VERSION 0.1.0 LANGUAGES C CXX OBJC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_OSX_DEPLOYMENT_TARGET "15.0" CACHE STRING "Minimum iOS version")

include(FetchContent)

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  SDL2
  URL https://github.com/libsdl-org/SDL/releases/download/release-2.32.10/SDL2-2.32.10.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(SDL2)

add_library(
  formfactor_core
  STATIC
  ../src/component.cpp
  ../src/hash.cpp
  ../src/policy_trust.cpp
  ../src/key_authority.cpp
  ../src/source_authority.cpp
  ../src/catalog.cpp
  ../src/catalog_filter.cpp
  ../src/quantity.cpp
  ../src/circuit.cpp
  ../src/digital.cpp
  ../src/spice.cpp
  ../src/stackup.cpp
  ../src/layout.cpp
  ../src/impedance.cpp
  ../src/return_path.cpp
  ../src/pdn.cpp
  ../src/decoupling.cpp
)
target_include_directories(formfactor_core PUBLIC ../include)

add_executable(FormFactor MACOSX_BUNDLE ../src/workbench.cpp)

if(TARGET SDL2::SDL2main)
  target_link_libraries(FormFactor PRIVATE formfactor_core SDL2::SDL2main)
else()
  target_link_libraries(FormFactor PRIVATE formfactor_core)
endif()

if(TARGET SDL2::SDL2-static)
  target_link_libraries(FormFactor PRIVATE SDL2::SDL2-static)
elseif(TARGET SDL2::SDL2)
  target_link_libraries(FormFactor PRIVATE SDL2::SDL2)
else()
  message(FATAL_ERROR "SDL2 target was not created")
endif()

set_target_properties(
  FormFactor
  PROPERTIES
    OUTPUT_NAME "FormFactor"
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_BUNDLE_NAME "FormFactor"
    MACOSX_BUNDLE_GUI_IDENTIFIER "tech.bitcoinversus.formfactor"
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "tech.bitcoinversus.formfactor"
    XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
    XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "15.0"
    XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO"
)
