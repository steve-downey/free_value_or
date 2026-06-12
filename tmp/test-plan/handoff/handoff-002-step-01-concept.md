# Handoff 002 — after Step 01 (nullable concept coverage)

**Author:** Step 01 agent · **Date:** 2026-06-12 · **Branch merged:** `test/step-01`

## What I did

- **`tests/beman/free_value_or/concept.test.cpp`** (new file, 81 lines):
  - Double-include guard idempotency check (two `#include <beman/free_value_or/value_or.hpp>`)
  - Positive `static_assert(fvo::nullable<X>)` for every type in the test matrix:
    `std::optional<int>`, `std::optional<std::string>`, `const std::optional<int>`,
    `std::expected<int,int>`, `std::expected<std::string,int>`,
    `int*`, `const int*`, `double*`,
    `std::shared_ptr<int>`, `std::unique_ptr<int>`,
    `fvo_opt::optional<int&>`, `fvo_opt::optional<const int&>` (gated on FVO_HAS_OPTIONAL_REF)
  - Negative `static_assert(!fvo::nullable<X>)` for: `int`, `double`, `bool`,
    `std::string`, `std::vector<int>`, `bool_only`, `deref_only`, `nonconst_nullable`,
    `void`, `std::nullptr_t`
  - One `TEST_CASE` that spot-checks the same facts at runtime so ctest sees a result
- **`tests/beman/free_value_or/CMakeLists.txt`** — added `fvo_add_test(beman.free_value_or.tests.concept concept.test.cpp)`

## Build & test commands that actually worked

```bash
cmake -B build -S . -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=./infra/cmake/use-fetch-content.cmake
cmake --build build
ctest --test-dir build --output-on-failure
```

Result: 5/5 tests pass from both the worktree and the main checkout.
New test: `nullable concept: static assertions hold at compile time` — PASSED.

## Toolchain / standard facts I confirmed

No new toolchain facts beyond what Step 00 established. All static_asserts compiled
cleanly under GCC 15.2 / C++23.

## Gotchas / things that bit me

None. The step was straightforward — include-guard, static_asserts, one TEST_CASE,
one CMakeLists.txt line.

## Concept behavior findings (definitive list for this toolchain)

**POSITIVE (satisfies `nullable`):**
- `std::optional<int>`, `std::optional<std::string>`, `const std::optional<int>`
- `std::expected<int,int>`, `std::expected<std::string,int>` — YES, expected satisfies
  nullable: both `bool(e)` and `*e` compile on a `const expected`.
- `int*`, `const int*`, `double*`
- `std::shared_ptr<int>`, `std::unique_ptr<int>`
- `fvo_opt::optional<int&>`, `fvo_opt::optional<const int&>` (via beman::optional vendored lib)

**NEGATIVE (does NOT satisfy `nullable`):**
- `int`, `double`, `bool` — have contextual bool but no `operator*`
- `std::string`, `std::vector<int>` — no `operator bool` / no `operator*`
- `bool_only` — has `operator bool`, missing `operator*`
- `deref_only` — has `operator*`, missing contextual bool
- `nonconst_nullable` — both operators exist but are non-const; concept checks `const T`
- `void` — no operators at all
- `std::nullptr_t` — no `operator bool` or `operator*`

**Key subtlety confirmed:** `nonconst_nullable` does NOT satisfy `nullable` because the
concept body is `requires(const T t) { bool(t); *(t); }` — the `const T` parameter
means non-const member functions are inaccessible. This is intentional: nullable types
must be inspectable through a const ref (you can't mutate the container while observing it).

## Issues found (if any)

None.

## State of `main`

5/5 tests pass from the main checkout. All prior tests still green.

## What the next agent (Step 02) should know

- The definitive list above is the ground truth for what IS and ISN'T nullable on this
  toolchain. Step 02 (`value_or` runtime) should pick its runtime test cases from the
  positive list: `optional<int>`, `expected<int,int>`, raw `int*`, `shared_ptr<int>`,
  `unique_ptr<int>`, and `fvo_opt::optional<int&>` (gated).
- `std::expected<T,E>` DOES satisfy nullable — confirmed. Include it in runtime tests.
- `nonconst_nullable` does NOT satisfy nullable — cannot be passed to `value_or`/etc.
  (constraint failure at the call site).
- The test exe is named `beman.free_value_or.tests.concept` — don't collide with it.
- Use `fvo_add_test(beman.free_value_or.tests.value_or_full value_or.test.cpp)` or a
  new name for Step 02's exe (the placeholder `beman.free_value_or.tests.value_or`
  should be replaced per Step 02's instructions).
