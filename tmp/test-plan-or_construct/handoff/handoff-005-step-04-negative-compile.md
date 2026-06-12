# Handoff 005 — after Step 04 (negative-compile)

**Author:** Step 04 agent · **Date:** 2026-06-12 · **Branch merged:** `or_construct/step-04`

## What I did
- Added three WILL_FAIL TUs in `tests/beman/free_value_or/`:
  - `or_construct_non_nullable_fail.cpp` — pack overload: `or_construct(5, 3)`, int is not nullable.
  - `or_construct_initlist_non_nullable_fail.cpp` — init-list overload: `or_construct(std::string{"a"}, {1, 2, 3})`, std::string is not nullable.
  - `or_construct_payload_not_convertible_fail.cpp` — explicit Ret: `or_construct<int>(optional<string>{}, 0)`, string not convertible to int via static_cast.
- Registered all three in `tests/beman/free_value_or/CMakeLists.txt` via `fvo_add_compile_fail_test` in a commented `or_construct Step 04` block.

## Build & test commands that actually worked
```bash
cd /home/sdowney/src/free_value_or/oc-step-04
cmake -B build -S . -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=./infra/cmake/use-fetch-content.cmake
cmake --build build
ctest --test-dir build --output-on-failure
```
Result: **93/93 tests passed** (90 prior + 3 new `or_construct` fail tests). Also verified from the main checkout after merge: 93/93 green.

## Toolchain / standard facts I confirmed
- g++ 15.2.0, C++23 — no changes from inherited toolchain facts.
- Constraint failure diagnostic: `"no matching function for call to"` — confirmed for both pack and init-list overloads. Two-overload candidate set doesn't change the message vs. single-overload sibling functions.
- Payload-not-convertible diagnostic: `"invalid .*static_cast"` — confirmed exact wording is `"invalid 'static_cast' from type 'std::__cxx11::basic_string<char>' to type 'int'"` as documented in the step file.

## Gotchas / things that bit me
- The sanity check: when temporarily substituting `or_construct(std::optional<int>{}, 3)` (a valid call) without `#include <optional>`, the build still failed (wrong error). Added the include to get a clean compile, confirmed ctest then reported failure (build succeeded → test fails). Restored original.
- The init-list overload TU uses `std::string{"a"}` as the first arg and `{1, 2, 3}` as the list — the string has no `int` element type, so this is purely a non-nullable rejection. The diagnostic is the same `"no matching function for call to"`.

## Issues found (if any)
- None. Header is correct; both overloads enforce the nullable constraint as expected. Did NOT modify the header.

## State of `main`
- **93/93** tests pass from the main checkout (90 prior + 3 new fail tests).
- No targets excluded or skipped.

## What the next agent (Step 05) should know
- Step 05 (`steps/step-05-constexpr.md`) covers `constexpr` constant-evaluation of both `or_construct` overloads.
- Per CHECKLIST.md toolchain facts: `std::expected`, `std::optional`, and their ops are all `constexpr` at C++23 — confirmed by the sibling constexpr step.
- `std::unique_ptr` / `std::shared_ptr` are NOT usable in constant expressions — stick to `optional`, `expected`, and raw pointers.
- The init-list overload may not be constexpr-testable with `initializer_list<int>` depending on the standard — check whether `constexpr` contexts allow non-constexpr init-list items; if not, document as a known limitation.
- Build baseline: 93/93 green; worktree naming: `git worktree add -b or_construct/step-05 ../oc-step-05 main`.
