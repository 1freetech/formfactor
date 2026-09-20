cmake_minimum_required(VERSION 3.20)
project(FormFactorVisualLab VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
set(SUPPORT_FILEFORMAT_FLAC OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  raylib
  GIT_REPOSITORY https://github.com/raysan5/raylib.git
  GIT_TAG 6.0
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(raylib)

add_executable(formfactor_visual_lab main.cpp)
target_link_libraries(formfactor_visual_lab PRIVATE raylib)

if(MSVC)
  target_compile_options(formfactor_visual_lab PRIVATE /W4)
else()
  target_compile_options(formfactor_visual_lab PRIVATE -Wall -Wextra -Wpedantic)
endif()
