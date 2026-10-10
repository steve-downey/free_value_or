# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a [Beman project](https://github.com/bemanproject/beman) implementing free function overloads of `value_or`, `reference_or`, and `or_invoke` for any nullable type — a C++ standards proposal library. It is a header-only INTERFACE library by default, with optional C++ module support.

The implementation lives in `include/beman/free_value_or/value_or.hpp` under namespace `beman::free_value_or`. The public umbrella header is `include/beman/free_value_or/free_value_or.hpp`.

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
  free_value_or.hpp        # Umbrella header: `import beman.free_value_or;` or includes value_or.hpp
  free_value_or.cppm       # Module interface unit (wraps the umbrella header)
  value_or.hpp             # Implementation: beman::free_value_or::{value_or, reference_or, or_invoke, or_construct}
  beman_optional.hpp       # Opt-in: enable_borrowed_nullable for beman::optional::optional<T&>
  beman_expected.hpp       # Opt-in: enable_borrowed_nullable for beman::expected::expected<T&, E>
  config.hpp               # Module detection macros
  config_generated.hpp.in  # CMake-generated module flag

include/beman/{optional,expected}/   # Vendored git subtrees (see cmake/vendored-libs.cmake)

tests/beman/free_value_or/
  *.test.cpp               # Catch2 tests (fvo_add_test)
  *_fail.test.cpp          # Compile-fail tests (fvo_add_compile_fail_test; diagnostic regex in CMakeLists.txt)
  test_types.hpp           # Nullable fixtures + `fvo = beman::free_value_or` alias used by tests

examples/
  free_value_or.cpp        # Usage example (via the umbrella header)
```

Wiring detail: in a textual build the umbrella `free_value_or.hpp` includes `value_or.hpp`; with `BEMAN_FREE_VALUE_OR_USE_MODULES` it becomes `import beman.free_value_or;`, and standard headers must be included *before* it (GCC cannot merge textual std declarations after the import). Most tests therefore include `<beman/free_value_or/value_or.hpp>` directly; `public_header.test.cpp` and the example exercise the umbrella. The `beman_optional.hpp` / `beman_expected.hpp` adapters key off the `BEMAN_FREE_VALUE_OR_VALUE_OR_HPP` include guard to decide whether to include the vendored headers textually.

The `nullable` concept (in `value_or.hpp`) requires `bool(t)` and `*(t)` to be valid. All functions work on any type satisfying this concept — `std::optional`, `std::expected`, raw pointers, smart pointers, etc. In every case the "value" side of the result type is computed from `deref_t<T>` (`decltype(*std::declval<T>())`), which, unlike `std::iter_reference_t`, keeps the nullable's value category, so an expiring owning nullable yields an expiring payload.

- `value_or(m, u)` — returns `*m` if truthy, else `u`; result type via `std::common_type_t<deref_t<T>, U&&>`. `u` is a forwarding reference and is always evaluated (eager fallback).
- `reference_or(m, u)` — same but via `std::common_reference_t`; requires `borrowed_nullable<T>` (an lvalue, or an rvalue whose type opts in via `enable_borrowed_nullable`), and `static_assert`s using `reference_constructs_from_temporary_v` reject dangling references at compile time.
- `or_invoke(m, invocable)` — returns `*m` if truthy, else calls `invocable()`; result type via `common_type_t<deref_t<T>, invoke_result_t<I>>`.
- `or_construct<Ret>(m, args...)` — returns `*m` if truthy, else constructs `R(args...)`. Template param `Ret` defaults to `void`, in which case `R = remove_cvref_t<deref_t<T>>`. A second overload takes a leading `std::initializer_list`.

## Tests

Tests use **Catch2** (not GoogleTest). The `lockfile.json` / `vcpkg.json` pin the dependency versions for FetchContent.

## Beman Standard Compliance

`beman-tidy` enforces compliance with the [Beman Standard](https://github.com/bemanproject/beman/blob/main/docs/beman_standard.md). Run it via `.beman-tidy.yaml` in the root. All source files use `SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception`.
