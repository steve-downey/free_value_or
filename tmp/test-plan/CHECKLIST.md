# Checklist — running state

The **first unbox box is the current step.** Tick a box only after the step's branch is
merged into `main` and the suite builds from the main checkout.

- [x] **Step 00** — Test harness (CMake targets @ C++23 + C++26 variant, `test_types.hpp`,
      negative-compile CMake helper, smoke build) — `steps/step-00-harness.md`
- [x] **Step 01** — `nullable` concept static_assert coverage — `steps/step-01-concept.md`
- [x] **Step 02** — `value_or` runtime + return-type + value categories — `steps/step-02-value_or.md`
- [x] **Step 03** — `reference_or` reference semantics — `steps/step-03-reference_or.md`
- [x] **Step 04** — Negative-compile (dangling + non-nullable arg) — `steps/step-04-negative-compile.md`
- [ ] **Step 05** — `or_invoke` results + laziness — `steps/step-05-or_invoke.md`
- [ ] **Step 06** — `constexpr` constant-evaluation tests — `steps/step-06-constexpr.md`
- [ ] **Step 07** — `optional<T&>` (C++26) + finalize/consolidate — `steps/step-07-optional-ref-and-finalize.md`

## Toolchain facts (Step 00 fills this in, later steps amend)

- Compiler / libstdc++ version: `g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0`; `clang++` NOT found
- C++23 build: `cmake -B build -S . -DCMAKE_CXX_STANDARD=23 -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=./infra/cmake/use-fetch-content.cmake && cmake --build build && ctest --test-dir build --output-on-failure` — **SUCCESS** (3/3 tests pass)
- C++26 build: `-std=c++26` flag is accepted by GCC 15.2. `std::optional<T&>` NOT available
  in libstdc++ (`__cpp_lib_optional = 202110`). **Resolved** by vendoring `beman::optional26`
  via `git subtree` into `vendored/beman.optional26/`. Tests now always have `FVO_HAS_OPTIONAL_REF=1`
  via CMake compile definition and link `beman::optional`.
- `std::expected` available at C++23: YES (verified in toolchain probe)

## Issues / suspected header bugs (do NOT fix the header; record here)

_(none yet)_

## Notes carried across steps

- Build command for all positive tests (use from any worktree):
  ```bash
  cmake -B build -S . -DCMAKE_CXX_STANDARD=23 \
    -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=./infra/cmake/use-fetch-content.cmake
  cmake --build build
  ctest --test-dir build --output-on-failure
  ```
- `fvo_add_test(name source)` helper in CMakeLists.txt: 3-line positive test add.
- `fvo_add_compile_fail_test(test_name source regex)` helper: adds OBJECT library +
  `cmake --build --target` ctest entry + `PASS_REGULAR_EXPRESSION` check.
- `test_types.hpp` defines `namespace fvo = smd::free_value_or;` — use `fvo::` everywhere.
- `test_types.hpp` defines `namespace fvo_opt = beman::optional;` when `FVO_HAS_OPTIONAL_REF=1`.
  Use `fvo_opt::optional<T&>` for reference optional tests (Step 07).
- The `nullable` concept checks `const T` — types with non-const `operator bool`/`operator*`
  do NOT satisfy it (see `nonconst_nullable` in `test_types.hpp`).
- vendored optional26: `git subtree pull --prefix=vendored/beman.optional26 <path> HEAD --squash`
  to update it later.
