# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a [Beman project](https://github.com/bemanproject/beman) implementing the free functions `value_or`, `reference_or`, `or_invoke`, and `or_construct` over any nullable type — the reference implementation for WG21 paper D4270 (currently R1, unified with P3413R0). It is a header-only INTERFACE library by default, with optional C++ module support.

The free functions live in `include/beman/free_value_or/value_or.hpp` under namespace `smd::free_value_or` (the rename to `beman::free_value_or` is pending upstreaming; tests reach it through the `fvo` alias in `test_types.hpp`). The paper's member half — `value_or_construct` and `value_or_else` on `optional` and `expected` — is implemented in the co-located vendored subtrees `include/beman/optional/` and `include/beman/expected/`, which are pulled from and pushed back to their upstream Beman repositories with `scripts/vendor-*.sh`.

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

Other available preset names: `llvm-debug`, `llvm-release`, `appleclang-debug`, `appleclang-release`, `msvc-debug`, `msvc-release`. Non-host presets are gated by a `condition` on `hostSystemName`, so only the presets matching the current OS are usable.

All `*-debug` presets build with `BEMAN_BUILDSYS_SANITIZER=MaxSan` (max sanitizer coverage); `*-release` presets build `RelWithDebInfo`. Every configure preset uses the Ninja generator, sets `CMAKE_CXX_STANDARD=20`, and pulls dependencies via `infra/cmake/use-fetch-content.cmake`. Build presets also run the `all_verify_interface_header_sets` target, which checks that public headers are self-contained.

The `.build/build-system/` directory has additional named ninja configurations (Asan, Tsan, Gcov, Perf, RelWithDebInfo) built on top of the build-system preset.

## CMake Options

- `BEMAN_FREE_VALUE_OR_BUILD_TESTS` — build tests (default ON when top-level)
- `BEMAN_FREE_VALUE_OR_BUILD_EXAMPLES` — build examples (default ON when top-level)
- `BEMAN_FREE_VALUE_OR_USE_MODULES` — provide as a C++ module (default OFF)

## Architecture

```
include/beman/free_value_or/
  free_value_or.hpp        # Umbrella header: imports the module or includes value_or.hpp
  free_value_or.cppm       # Module interface unit (BEMAN_FREE_VALUE_OR_USE_MODULES=ON)
  value_or.hpp             # Implementation: smd::free_value_or::{nullable, deref_t, value_or, reference_or, or_invoke, or_construct}
  config.hpp               # Module detection macros
  config_generated.hpp.in  # CMake-generated module flag

include/beman/optional/    # Vendored beman.optional, carrying value_or_construct / value_or_else
include/beman/expected/    # Vendored beman.expected, likewise, including the T&/E& specializations

tests/beman/free_value_or/
  *.test.cpp               # Catch2 tests, one file per function or axis (see its README.md)
  *_fail.cpp               # Negative-compilation units, matched against an expected diagnostic
  test_types.hpp           # Nullable fixtures + `fvo` namespace alias used by tests
tests/beman/optional/, tests/beman/expected/   # The vendored libraries' own suites, run as part of this project

examples/
  free_value_or.cpp        # Usage examples

papers/
  free_value_or.tex        # D4270 source; design.org and review-2026-09.org are the working notes
```

Wiring: the umbrella `free_value_or.hpp` includes `value_or.hpp` directly, or `import beman.free_value_or;` when modules are on; the module unit re-exports the umbrella. Either route reaches `smd::free_value_or`.

The `nullable` concept (in `value_or.hpp`) requires `bool(t)` and `*(t)` on a `const remove_reference_t<T>&` — observation must not need mutable access. All functions work on any type satisfying it: `std::optional`, `std::expected`, raw pointers, smart pointers, `optional<T&>`. The "value" side of every result type is `deref_t<T> = decltype(*std::declval<T>())`, the type `*m` yields with the nullable's own value category carried through: an rvalue `optional`/`expected` dereferences to `T&&` and the engaged path moves its payload; pointers (`T*`, `shared_ptr`, `unique_ptr`) and `optional<T&>` dereference to `T&` whatever their value category and copy. The split is containment (is the payload a subobject?), not ownership — `unique_ptr` owns and still yields `T&`.

- `value_or(m, u)` — returns `*m` if truthy, else `u`; result type via `std::common_type_t<deref_t<T>, U&&>`. `u` is a forwarding reference and always evaluated (eager fallback).
- `reference_or(m, u)` — same but via `std::common_reference_t`; two `static_assert`s using `std::reference_constructs_from_temporary_v` (polyfilled for C++20/MSVC) reject dangling references at compile time. When the common reference of unrelated types is not a reference, it degrades to a prvalue — the paper's one open LEWG question.
- `or_invoke(m, invocable)` — returns `*m` if truthy, else calls `std::forward<I>(invocable)()`; result type via `common_type_t<deref_t<T>, invoke_result_t<I>>`.
- `or_construct<Ret>(m, args...)` — returns `*m` if truthy, else constructs `R(args...)`. Template param `Ret` defaults to `void`, in which case `R = remove_cvref_t<deref_t<T>>`. A second overload takes a leading `std::initializer_list`. (In the header, `R` precedes the `Args` pack for MSVC.)

## Tests

Tests use **Catch2** (not GoogleTest). The `lockfile.json` / `vcpkg.json` pin the dependency versions for FetchContent.

## Beman Standard Compliance

`beman-tidy` enforces compliance with the [Beman Standard](https://github.com/bemanproject/beman/blob/main/docs/beman_standard.md). Run it via `.beman-tidy.yaml` in the root. All source files use `SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception`.
