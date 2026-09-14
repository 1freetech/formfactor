#!/usr/bin/env sh
set -eu

repo_root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
build_dir=${FORMFACTOR_CMAKE_BUILD_DIR:-"$repo_root/build-cmake"}
tool_dir="$repo_root/build/tooling/cmake"
cmake_version=${FORMFACTOR_CMAKE_VERSION:-4.4.3}

if command -v cmake >/dev/null 2>&1 && command -v ctest >/dev/null 2>&1; then
  run_cmake() { cmake "$@"; }
  run_ctest() { ctest "$@"; }
else
  if [ ! -x "$tool_dir/bin/cmake" ] || [ ! -x "$tool_dir/bin/ctest" ]; then
    echo "CMake/CTest not found; installing CMake $cmake_version under build/tooling/cmake"
    python3 -m pip install --disable-pip-version-check \
      --target "$tool_dir" "cmake==$cmake_version"
  fi

  run_cmake() {
    PYTHONPATH="$tool_dir${PYTHONPATH:+:$PYTHONPATH}" \
      "$tool_dir/bin/cmake" "$@"
  }
  run_ctest() {
    PYTHONPATH="$tool_dir${PYTHONPATH:+:$PYTHONPATH}" \
      "$tool_dir/bin/ctest" "$@"
  }
fi

run_cmake -S "$repo_root" -B "$build_dir" \
  -DFORMFACTOR_BUILD_FREELAB=OFF
run_cmake --build "$build_dir" --parallel "${CMAKE_BUILD_PARALLEL_LEVEL:-2}"
run_ctest --test-dir "$build_dir" --output-on-failure
