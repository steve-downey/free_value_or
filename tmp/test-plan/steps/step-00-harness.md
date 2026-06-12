# Step 00 — Test harness & shared fixtures

**Goal:** stand up everything later steps depend on, and *prove the toolchain is adequate*.
No behavioral assertions yet beyond a smoke test.

## Deliverables

1. **Toolchain probe (record findings, don't commit the probe).** Before touching CMake,
   determine empirically:
   - `g++ --version`, and whether `clang++` exists.
   - Does a TU compile at `-std=c++23` that calls all three functions on a
     `std::optional<int>`? (It must — header needs C++23.)
   - Does `-std=c++26` work, and does `std::optional<int&>` compile under it? Print
     `__cpp_lib_optional`. Does `<expected>` give a working `std::expected<int,int>`?

   Use a throwaway file in `/tmp` (not the repo). Write the results into
   `CHECKLIST.md` "Toolchain facts" and your handoff.

2. **`tests/beman/free_value_or/test_types.hpp`** — shared fixtures for all later steps:
   - A helper to build each *engaged* and *disengaged* nullable value for:
     `std::optional<T>`, `std::expected<T,E>`, raw pointer `T*` (engaged = `&obj`,
     disengaged = `nullptr`), `std::shared_ptr<T>`, `std::unique_ptr<T>`.
   - **Anti-model types** (must NOT be `nullable`):
     - `bool_only`: has `explicit operator bool() const` but no `operator*`.
     - `deref_only`: has `int& operator*() const` but no `operator bool`.
     - `nonconst_nullable`: has `operator bool`/`operator*` that are **non-const**
       (illustrates that `nullable` checks a `const T`).
   - Keep `optional<T&>` (C++26) behind
     `#if defined(__cpp_lib_optional) && __cpp_lib_optional >= 202506L` (use the real
     threshold you discover in the probe). Provide a macro like
     `FVO_HAS_OPTIONAL_REF` the later steps can `#if` on.
   - Header guard + SPDX line. No Catch2 dependency in this header (pure types/helpers) so
     it can be included by negative-compile TUs too.
   - **Define the rename-proofing alias** once, here, so every test reuses it:
     `namespace fvo = smd::free_value_or;  // rename point: smd:: -> beman::`. All later
     steps refer to `fvo::value_or` / `fvo::reference_or` / `fvo::or_invoke` / `fvo::nullable`
     rather than spelling `smd::` inline (the namespace becomes `beman::free_value_or` in the
     future "great renaming"). Negative-compile TUs that can't include this header carry the
     same one-line alias themselves.

3. **`tests/beman/free_value_or/smoke.test.cpp`** — a minimal Catch2 TU that includes
   `value_or.hpp` + `test_types.hpp`, and does a single engaged + disengaged
   `value_or(std::optional<int>{...}, 0)` `CHECK`. Purpose: prove the test target compiles
   and links at C++23.

4. **CMake wiring** in `tests/beman/free_value_or/CMakeLists.txt`:
   - Establish that test executables build at **C++23** (e.g.
     `target_compile_features(<tgt> PRIVATE cxx_std_23)`), without changing the library's
     own standard. Add a small helper (a `function()` or just a documented pattern) so
     later steps add a positive test exe in 3 lines.
   - Add the `smoke` executable via that helper, `catch_discover_tests` it.
   - Add a **C++26 test executable variant** mechanism (a second helper, or a parameter)
     for the `optional<T&>` step — even if it currently wraps the same smoke source. Only
     enable it when the compiler accepts `-std=c++26` (guard with a CMake check or a
     cache option `BEMAN_FREE_VALUE_OR_TESTS_CXX26`, default auto/off). Document how to
     turn it on in your handoff.
   - Establish the **negative-compile (WILL_FAIL) helper** now (used heavily in Step 04).
     Mirror the proven pattern from
     `~/src/transcode/main/tests/beman/transcode/CMakeLists.txt` (search `WILL_FAIL` /
     `PASS_REGULAR_EXPRESSION`): an `OBJECT` (or executable) library `EXCLUDE_FROM_ALL`,
     plus an `add_test` that invokes `cmake --build --target <tgt>` and a
     `set_tests_properties(... PASS_REGULAR_EXPRESSION "<diag regex>")`. Wrap it in a CMake
     `function(fvo_add_compile_fail_test name source regex)` so Step 04 is trivial. Add
     **one** trivial self-check use of it (e.g. a TU that `static_assert(false-ish)` or
     references `nullable<int>` in a `requires`-failing call) to prove the harness reports
     compile-fails correctly — then you know Step 04's machinery works.

   Keep the existing `beman.free_value_or.tests.value_or` target building (do not delete
   `value_or.test.cpp` yet — Step 02 replaces it).

## Build & verify

```bash
# from the worktree
cmake -B build -S . -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=./infra/cmake/use-fetch-content.cmake
cmake --build build
ctest --test-dir build --output-on-failure
```
(FetchContent pulls Catch2 with no system install needed — confirm it does. If the project
prefers vcpkg, note that; FetchContent is the zero-setup path.)

The negative-compile self-check test must **pass** (i.e. the bad TU failed to compile and
the regex matched).

## Done criteria
- `smoke` test passes at C++23. Toolchain facts recorded in CHECKLIST + handoff.
- `test_types.hpp` exists and is includable.
- Negative-compile CMake helper exists and its self-check passes.
- Merged to `main`; suite builds from the main checkout.

## Notes for your handoff
- Exact configure/build/ctest commands that worked (the next 7 agents will copy them).
- The real `__cpp_lib_optional` threshold and whether C++26 / `optional<T&>` is usable.
- The exact diagnostic text GCC emits for a failed `nullable` constraint and for a failed
  `reference_constructs_from_temporary_v` static_assert (copy a snippet) — Step 04 needs
  these to write `PASS_REGULAR_EXPRESSION`s.
