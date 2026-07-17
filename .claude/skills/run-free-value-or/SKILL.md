---
name: run-free-value-or
description: Build, run, and smoke-test the beman.free_value_or C++ header-only library. Use when asked to run, build, compile, test, or smoke-test free_value_or / value_or, verify a change to value_or.hpp compiles and behaves, or run its Catch2 test suite.
---

# Run beman.free_value_or

`free_value_or` is a **header-only C++23 library** (a Beman project). There is
no app, server, or GUI — the entire implementation is
`include/beman/free_value_or/value_or.hpp` (namespace `smd::free_value_or`:
`value_or`, `reference_or`, `or_invoke`, `or_construct`). "Running" it means
compiling a translation unit against the header and executing checks.

Three verified ways to drive it, fastest first. **Paths below are relative to
the unit root** (`<unit>/` = this repo root, where `CMakeLists.txt` lives).

## Prerequisites

All were already present in this container; versions that worked:

- **C++23 compiler** — `g++` 13.3 *or* `clang++` 23 (either works for the driver).
  On a clean Ubuntu: `sudo apt-get install -y g++ cmake ninja-build`.
- For `make` / preset paths only: `cmake` 4.3, `ninja`, and (for `make`) `uv`
  + `ccache`. Check with `command -v g++ clang++ cmake ninja uv ccache`.
- The full test suite also fetches **Catch2** from GitHub (network) and uses the
  vendored **beman::optional** submodule under `vendored/` (already populated).

## Run (agent path) — the driver

The driver compiles `smoke.cpp` (next to this file) against `include/` at C++23
and runs it. It needs **only a compiler** — no CMake, no network, no Catch2.
This is the fast inner loop for changes to `value_or.hpp`.

```bash
.claude/skills/run-free-value-or/driver.sh            # uses g++
.claude/skills/run-free-value-or/driver.sh clang++    # pick the compiler
```

Expected last line:

```
free_value_or smoke: ALL CHECKS PASSED
```

The smoke program exercises `value_or` / `reference_or` / `or_invoke` /
`or_construct` (incl. the `initializer_list` overload) across `std::optional`,
`std::expected`, raw pointer, `shared_ptr`, and `unique_ptr`, plus
`common_type` promotion and lazy-fallback behavior. Edit
`.claude/skills/run-free-value-or/smoke.cpp` to add checks for new behavior.

Equivalent one-liner if you want to drive the compile yourself:

```bash
g++-13 -std=c++23 -Wall -Iinclude .claude/skills/run-free-value-or/smoke.cpp -o /tmp/fvo_smoke && /tmp/fvo_smoke
```

## Run the full test suite (106 Catch2 tests)

### Canonical dev path — the Makefile

`make test` is the project's real workflow: it configures
`.build/build-system/` (Ninja Multi-Config, **Asan** config by default via
`uv run cmake`/`ccache`), builds the `all` target, and runs `ctest`.

```bash
make test
```

Ends with `100% tests passed, 0 tests failed out of 106`. Other useful goals
(`make help` lists all): `make compile`, `make ctest`, `make lint`
(pre-commit), `make coverage`, `make docs`.

### Portable path — CMake presets

The full workflow (configure + build + header-verify + test) passes:

```bash
cmake --workflow --preset gcc-debug
```

Ends with `100% tests passed ... out of 106`. (First run configures the preset
and git-clones Catch2 into `build/gcc-debug/_deps`.) Other presets:
`gcc-release`, `llvm-debug`, `llvm-release`.

## Gotchas

- **`value_or.hpp` is C++20-clean; C++17 is not supported.** The header uses
  concepts, `iter_reference_t`, `common_reference_t`, `remove_cvref_t` — all
  C++20. It also needs the P2255 dangling-reference trait
  `std::reference_constructs_from_temporary_v` (C++23), which is polyfilled via
  the `__reference_constructs_from_temporary` builtin so the header still
  compiles (with the check active) at C++20 and on C++23 stdlibs that predate
  the trait (libstdc++ < 13, Apple libc++). The header-set verify at C++20 and
  the `make compile-headers` target both pass because of this. The *test suite*
  and the driver's `smoke.cpp` additionally use `std::expected` (C++23), so
  compile those with `-std=c++23`.
- **The umbrella header does not include the implementation.**
  `free_value_or.hpp` includes the `todo.hpp` scaffold (namespace
  `beman::free_value_or`), *not* `value_or.hpp`. Include
  `<beman/free_value_or/value_or.hpp>` directly and use `smd::free_value_or`
  (that is exactly what the tests and this driver do).
- **`make` runs cmake/ctest through `uv run`** and uses a `ccache` compiler
  launcher; both must be on PATH or the Makefile paths fail.

## Troubleshooting

- `'concept' only available with '-std=c++20'` → you are compiling below C++20.
  This library requires C++20 minimum; C++17 cannot work.
- `Could NOT find Catch2` / git clone errors during a preset or `make` build →
  the suite needs network to FetchContent Catch2. The `driver.sh` path avoids
  this entirely; use it when offline.
