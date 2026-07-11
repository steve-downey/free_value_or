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

⚠️ `cmake --workflow --preset gcc-debug` **fails here** on the
`all_verify_interface_header_sets` step (see Gotchas). Build the `all` target
explicitly instead, then test:

```bash
cmake --build --preset gcc-debug --target all
ctest --preset gcc-debug
```

Also ends with `100% tests passed ... out of 106`. (First run configures the
preset and git-clones Catch2 into `build/gcc-debug/_deps`.) Other presets:
`gcc-release`, `llvm-debug`, `llvm-release`.

## Gotchas

- **`cmake --workflow --preset *` fails on `all_verify_interface_header_sets`
  with gcc/clang whose libstdc++ gates the trait.** That target compiles each
  public header standalone at **C++20** (the root preset sets
  `CMAKE_CXX_STANDARD=20`), but `value_or.hpp` uses
  `std::reference_constructs_from_temporary_v`, a **C++23** trait. gcc-13's
  libstdc++ only defines it at `-std=c++23`, so the C++20 header-verify errors
  with *"reference_constructs_from_temporary_v is not a member of std"*. The
  library's own targets build fine because they set `cxx_std_23`. Workaround:
  build `--target all` (above), which skips the verify target. `make
  compile-headers` hits the same wall for the same reason.
- **The header requires C++23, not C++20.** `value_or.hpp` pulls in
  `std::expected`, `std::iter_reference_t`, and the temporary-binding trait.
  Always compile consumers with `-std=c++23`.
- **The umbrella header does not include the implementation.**
  `free_value_or.hpp` includes the `todo.hpp` scaffold (namespace
  `beman::free_value_or`), *not* `value_or.hpp`. Include
  `<beman/free_value_or/value_or.hpp>` directly and use `smd::free_value_or`
  (that is exactly what the tests and this driver do).
- **`make` runs cmake/ctest through `uv run`** and uses a `ccache` compiler
  launcher; both must be on PATH or the Makefile paths fail.

## Troubleshooting

- `reference_constructs_from_temporary_v is not a member of 'std'` → you (or the
  verify target) are compiling at C++20. Use `-std=c++23`, or for the preset
  build use `--target all` instead of the full workflow.
- `Could NOT find Catch2` / git clone errors during a preset or `make` build →
  the suite needs network to FetchContent Catch2. The `driver.sh` path avoids
  this entirely; use it when offline.
