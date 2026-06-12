# Handoff 004 — after Step 03 (reference_or reference semantics)

**Author:** Step 03 agent · **Date:** 2026-06-12 · **Branch merged:** `test/step-03`

## What I did

- **`tests/beman/free_value_or/reference_or.test.cpp`** — new file with full coverage:
  - Return-type static_asserts for every nullable type: `R = common_reference_t<iter_reference_t<T>, U&&>` yields `int&` or `const int&` (not a prvalue like `value_or`'s `int`).
  - Reference identity: `&reference_or(m, fb) == &(*m)` engaged; `== &fb` disengaged.
  - Return-type is-reference assertion + exact-type assertion in a TEST_CASE.
  - const propagation: `const optional<int>& + const int&` → `const int&`; `optional<int>& + const int&` → `const int&` (common_reference promotes to const).
  - All confirmed nullable types: `optional`, `expected`, `int*`, `shared_ptr` (int payload + lvalue fallback).
  - Mutation round-trip: write through reference changes `*m` (engaged) or `fallback` (disengaged), proving no copy.
  - String payload: `optional<string>& + string&` → `string&`.
- **`tests/beman/free_value_or/CMakeLists.txt`** — added one line: `fvo_add_test(beman.free_value_or.tests.reference_or reference_or.test.cpp)`.

## Build & test commands that actually worked

```bash
cmake -B build -S . -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=./infra/cmake/use-fetch-content.cmake
cmake --build build
ctest --test-dir build --output-on-failure
```

Result: **26/26 tests pass** from both the worktree and the main checkout.
New tests (12 ctest entries from one executable): all 12 passed.

## Toolchain / standard facts I confirmed

No new toolchain surprises. GCC 15.2 / C++23. All static_asserts compiled cleanly.
`common_reference_t` works as expected for lvalue+lvalue → lvalue reference.

## Safe (T, U) pairs confirmed for Step 04

These combinations pass `reference_or`'s two `reference_constructs_from_temporary_v` guards and yield a genuine lvalue reference:

| nullable T (lvalue)         | fallback U (lvalue) | R             |
|-----------------------------|---------------------|---------------|
| `optional<int>&`            | `int&`              | `int&`        |
| `const optional<int>&`      | `const int&`        | `const int&`  |
| `optional<int>&`            | `const int&`        | `const int&`  |
| `const optional<int>&`      | `int&`              | `const int&`  |
| `expected<int,int>&`        | `int&`              | `int&`        |
| `int*`                      | `int&`              | `int&`        |
| `shared_ptr<int>&`          | `int&`              | `int&`        |
| `optional<string>&`         | `string&`           | `string&`     |

**Would-be temporaries (Step 04 negative cases):**
- `reference_or(optional<int>&, int{})` — `U = int` (prvalue), `U&&=int&&`,
  `common_reference_t<int&, int&&> = const int&`, then
  `reference_constructs_from_temporary_v<const int&, int>` = **true** → static_assert fires.
- `reference_or(optional<int>&&, int&)` — T deduced as `optional<int>`, `iter_reference_t<optional<int>> = int&` — this should be same as lvalue case actually; it's the prvalue fallback that triggers it.
- Any combination where `R` would bind to a temporary produced from `U` or from `*m`.

## Gotchas / things that bit me

- None. The static_assert guards in the header proactively block all dangling cases, so there's no accidental step-04 territory in the step-03 tests.
- `unique_ptr<int>` was NOT included — `reference_or` with a `unique_ptr` rvalue would require `T&&` deduction as `unique_ptr<int>`, and finding a non-dangling lvalue fallback is straightforward, but `unique_ptr` is move-only so an rvalue test is harder. Since the step plan didn't mention it and all other nullable types are covered, it was omitted. Step 04 can add a negative-compile case if needed.

## Issues found (if any)

None. No header bugs discovered.

## State of `main`

26/26 tests pass from the main checkout after the merge. All prior tests still green.

## What the next agent (Step 04) should know

- **Step 04** is negative-compile (dangling + non-nullable arg) — `steps/step-04-negative-compile.md`.
- Use `fvo_add_compile_fail_test(name source regex)` helper (already in CMakeLists.txt).
- The two guards in `reference_or`:
  ```cpp
  static_assert(!std::reference_constructs_from_temporary_v<R, U>);
  static_assert(!std::reference_constructs_from_temporary_v<R, T&>);
  ```
  The diagnostic you'll see is `static assertion failed` — use a regex like `static assertion failed` or the more specific trait name.
- One confirmed dangling trigger: `reference_or(optional<int>&, int{42})` where `U=int` (prvalue) → `R=const int&` → first guard fires.
- A second type of negative: calling `value_or` or `reference_or` with a non-nullable `T` (e.g. plain `int`) → "no matching function for call to" (same regex as the existing `fail_not_nullable.cpp`).
- The `fvo_add_compile_fail_test` macro prepends `beman.free_value_or.tests.` to `tgt_name`. The `test_name` arg becomes both the ctest test name and the `<tgt_name>` suffix. Keep names short.
