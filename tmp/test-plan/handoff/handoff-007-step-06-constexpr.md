# Handoff 007 — after Step 06 (constexpr constant-evaluation tests)

**Author:** Step 06 agent · **Date:** 2026-06-12 · **Branch merged:** `test/step-06`

## What I did

Added `tests/beman/free_value_or/constexpr.test.cpp` (new exe via `fvo_add_test`) and a
6-line CMakeLists.txt entry for Step 06.

### Coverage in constexpr.test.cpp

All proofs are `static_assert`s at file scope — they force constant evaluation:

**value_or:**
- `optional<int>{7}` engaged → 7
- `optional<int>{}` disengaged → 5
- `expected<int,int>{42}` engaged → 42
- `expected<int,int>{unexpected(1)}` disengaged → 99
- `const int*` pointing at a `constexpr` static → raw pointer in constant context works
- `nullptr` (raw pointer disengaged)

**reference_or:**
- `optional<int>{ce_a}` engaged → ce_a value (10)
- `optional<int>{}` disengaged → ce_b value (20)

**or_invoke:**
- `optional<int>{9}` engaged → 9 (lambda not called)
- `optional<int>{}` disengaged → lambda returns 4
- `expected<int,int>{unexpected(0)}` disengaged → lambda returns 77

Plus a trivial `TEST_CASE("constexpr static_asserts compile and hold") { SUCCEED(); }` so ctest
has a runnable entry.

## Build & test commands that actually worked

```bash
cmake -B build -S . -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=./infra/cmake/use-fetch-content.cmake
cmake --build build
ctest --test-dir build --output-on-failure
```

Result: **46/46 tests pass** (1 new constexpr test + 45 existing).

## Toolchain / standard facts I confirmed

- `std::optional` ops are `constexpr` in C++23: YES
- `std::expected` ops are `constexpr` in C++23: YES
- Raw `const int*` pointing at a `constexpr` static works in constant expressions: YES
- `constexpr` lambdas (stateless `[] { return N; }`) work in `static_assert` context at C++23: YES
- No new toolchain surprises; everything from prior handoffs still holds.

## Gotchas / things that bit me

None — the step was straightforward. All types targeted in the step file worked as specified.
`reference_or` in constant context returns the value (not a live reference), so comparing the
result to the expected integer value works directly in `static_assert`.

## Issues found

None. No header bugs.

## State of `main`

46/46 tests pass from the main checkout after the merge. All prior tests green.

## What the next agent (Step 07) should know

- **Step 07** is `optional<T&>` (C++26) + finalize/consolidate — `steps/step-07-optional-ref-and-finalize.md`.
- `beman::optional` is vendored and always available via `fvo_opt::optional<T&>` (see `test_types.hpp`).
  `FVO_HAS_OPTIONAL_REF=1` is injected by CMake for all positive tests.
- The `or_invoke` vs `reference_or` asymmetry noted in handoff-006 is still worth documenting:
  `or_invoke` uses `common_type_t` (yields a value), `reference_or` uses `common_reference_t` (can
  yield a reference). Step 07 consolidation should note this in the test README.
- 46 tests currently passing — do not break any of them.
