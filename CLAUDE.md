# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a [Beman project](https://github.com/bemanproject/beman) implementing free function overloads of `value_or`, `reference_or`, and `or_invoke` for any nullable type — a C++ standards proposal library. It is a header-only INTERFACE library by default, with optional C++ module support.

The implementation lives in `include/beman/free_value_or/value_or.hpp` under namespace `smd::free_value_or`. The `beman::free_value_or` namespace in `todo.hpp` is a scaffold placeholder for future content.

## Build Commands

Uses CMake presets. The build directory for the preset-based build is `.build/build-system/` (already configured); per-preset builds land in `build/<preset-name>/`.

```bash
# Full workflow (configure + build + test)
cmake --workflow --preset gcc-debug
cmake --workflow --preset gcc-release

# List all available presets
cmake --list-presets=workflow

# Build only (after configure)
cmake --build --preset gcc-debug

# Run tests only
ctest --preset gcc-debug

# Run a single test binary directly (after build)
./build/gcc-debug/tests/beman/free_value_or/beman.free_value_or.tests.value_or

# Manual configure with FetchContent (no external deps needed)
cmake -B build -S . \
  -DCMAKE_CXX_STANDARD=20 \
  -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=./infra/cmake/use-fetch-content.cmake
cmake --build build
ctest --test-dir build
```

Other available preset names: `llvm-debug`, `llvm-release`, `appleclang-debug`, `appleclang-release`.

The `.build/build-system/` directory has additional named ninja configurations (Asan, Tsan, Gcov, Perf, RelWithDebInfo) built on top of the build-system preset.

## CMake Options

- `BEMAN_FREE_VALUE_OR_BUILD_TESTS` — build tests (default ON when top-level)
- `BEMAN_FREE_VALUE_OR_BUILD_EXAMPLES` — build examples (default ON when top-level)
- `BEMAN_FREE_VALUE_OR_USE_MODULES` — provide as a C++ module (default OFF)

## Architecture

```
include/beman/free_value_or/
  free_value_or.hpp        # Umbrella header: selects module import or header include
  todo.hpp                 # Scaffold: beman::free_value_or namespace (placeholder)
  value_or.hpp             # Implementation: smd::free_value_or::{value_or, reference_or, or_invoke}
  config.hpp               # Module detection macros
  config_generated.hpp.in  # CMake-generated module flag

tests/beman/free_value_or/
  value_or.test.cpp        # Catch2 tests

examples/
  todo.cpp                 # Usage examples
```

The `nullable` concept (in `value_or.hpp`) requires `bool(t)` and `*(t)` to be valid. All three functions work on any type satisfying this concept — `std::optional`, raw pointers, smart pointers, etc.

- `value_or(m, u)` — returns `*m` if truthy, else `u`; result type via `std::common_type_t`
- `reference_or(m, u)` — same but via `std::common_reference_t`; static_asserts prevent dangling references
- `or_invoke(m, invocable)` — returns `*m` if truthy, else calls `invocable()`

## Tests

Tests use **Catch2** (not GoogleTest). The `lockfile.json` / `vcpkg.json` pin the dependency versions for FetchContent.

## Beman Standard Compliance

`beman-tidy` enforces compliance with the [Beman Standard](https://github.com/bemanproject/beman/blob/main/docs/beman_standard.md). Run it via `.beman-tidy.yaml` in the root. All source files use `SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception`.
