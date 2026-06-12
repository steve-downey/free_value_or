# Handoff 001 — after Step 00 (harness) + vendored beman::optional26

**Author:** Step 00 agent · **Date:** 2026-06-12 · **Branches merged:** `test/step-00`, `vendored/optional26`

## What I did

### Step 00 (initial harness)
- **`tests/beman/free_value_or/test_types.hpp`** — shared fixtures:
  - `namespace fvo = smd::free_value_or;` alias (rename-proofing; ALL tests use `fvo::`)
  - `FVO_HAS_OPTIONAL_REF` macro — set to 1 by CMake compile definition (see below)
  - `namespace fvo_opt = beman::optional;` when FVO_HAS_OPTIONAL_REF=1 (use for `optional<T&>`)
  - `NullableFixture<T>` with helpers for `optional<T>`, `expected<T,int>`,
    `shared_ptr<T>`, `unique_ptr<T>` (engaged + disengaged)
  - `raw_engaged<T>()` / `raw_disengaged<T>()` raw pointer helpers
  - Anti-model types: `bool_only`, `deref_only`, `nonconst_nullable`
- **`tests/beman/free_value_or/smoke.test.cpp`** — C++23 smoke test + optional<int&> smoke test
- **`tests/beman/free_value_or/fail_not_nullable.cpp`** — negative-compile self-check
- **`tests/beman/free_value_or/CMakeLists.txt`** — updated with:
  - `fvo_add_test(name source)` — 3-line positive test helper
  - `fvo_add_compile_fail_test(test_name source regex)` — OBJECT + cmake --build + PASS_REGULAR_EXPRESSION
  - Both helpers link `beman::optional` and inject `-DFVO_HAS_OPTIONAL_REF=1`

### Vendor addition (follow-up)
- **`vendored/beman.optional26/`** — `beman::optional26` added via `git subtree add --squash`
  from `/home/sdowney/src/Optional26/main` (upstream: `git@github.com:beman-project/Optional26`)
- **`vendored/CMakeLists.txt`** — minimal INTERFACE target `beman::optional` (no tests built)
- **`CMakeLists.txt`** — `add_subdirectory(vendored)` inserted before tests

## Build & test commands that actually worked
```bash
cmake -B build -S . -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=./infra/cmake/use-fetch-content.cmake
cmake --build build
ctest --test-dir build --output-on-failure
```
Result: 4/4 tests pass:
1. `value_or` (existing placeholder) — PASSED
2. `smoke: value_or with optional<int>` — PASSED
3. `smoke: value_or with beman::optional<int&>` — PASSED (new, vendor test)
4. `fvo.fail.not_nullable` (negative-compile self-check) — PASSED

## Toolchain / standard facts I confirmed
- **Compiler:** `g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0`
- **clang++:** NOT found on PATH
- **C++23:** Fully functional. Header's `reference_constructs_from_temporary_v` requires C++23.
- **std::optional<T&>:** NOT available in this libstdc++ (`__cpp_lib_optional = 202110`).
  **Resolved:** vendored `beman::optional` provides it at C++23.
- **`std::expected`:** Available at C++23.
- **FVO_HAS_OPTIONAL_REF:** Always 1 in all test targets (injected by CMake). Tests
  that include `test_types.hpp` get `fvo_opt::optional<T&>` available.

## Gotchas / things that bit me
- **cmake_dependent_option requires `include(CMakeDependentOption)`** — easy to forget.
- **PASS_REGULAR_EXPRESSION, not WILL_FAIL:** The cmake --build exits 1 but ctest treats
  it as a pass when output matches the regex.
- **Catch2 test discovery race:** If first build fails, stale `NOT_BUILT-XXXX` test
  entries appear. Rebuild and re-run ctest; the second run is correct.
- **vendored/beman.optional26 contains the full repo** (papers, CI, etc.) — only
  `include/beman/optional/` matters for tests; the rest is inert.

## Diagnostic text for GCC 15.2 (for Step 04's PASS_REGULAR_EXPRESSION)
**nullable constraint failure** (e.g., calling `value_or(int, 0)`):
```
error: no matching function for call to 'value_or(int&, int)'
  note: template constraint failure for ... requires  nullable<T>
  error: constraints not satisfied
```
Key regex: `"no matching function for call to"` or `"constraints not satisfied"`.

**reference_or dangling static_assert** (e.g., `reference_or(optional<int>{42}, 0)`):
```
error: static assertion failed
  note: '!(bool)std::reference_constructs_from_temporary_v<const int&, int>' evaluates to false
```
Key regex: `"static assertion failed"` or `"reference_constructs_from_temporary_v"`.

The dangling check fires when passing an rvalue int to `reference_or(optional<int>&, ...)`:
- `U = int` (rvalue → U&& = int&&)
- `R = common_reference_t<int&, int&&> = const int&` (IS a reference)
- `reference_constructs_from_temporary_v<const int&, int>` = true → static_assert fires

## Issues found (if any)
None.

## State of `main`
Suite builds and all 4 tests pass from the main checkout.

## What the next agent (Step 01) should know
- Include `test_types.hpp` for the `fvo::` alias, anti-model types, `NullableFixture<T>`,
  and `fvo_opt::optional<T&>`.
- Use `fvo_add_test(beman.free_value_or.tests.concept concept.test.cpp)` to add Step 01's exe.
- Anti-model types for `static_assert(!fvo::nullable<T>)`:
  - `bool_only`, `deref_only`, `nonconst_nullable` (from test_types.hpp)
  - Also: `int`, `std::string`, `std::vector<int>` (none nullable)
- Positive nullable models: `std::optional<int>`, `std::expected<int,int>`,
  `int*`, `std::shared_ptr<int>`, `std::unique_ptr<int>`, `fvo_opt::optional<int&>`
- `std::expected<T,E>` DOES satisfy `nullable` — `bool(e)` and `*e` both work.
- To update the vendored optional later:
  `git subtree pull --prefix=vendored/beman.optional26 /home/sdowney/src/Optional26/main HEAD --squash`
