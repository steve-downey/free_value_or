# Handoff 001 — after Step 00 (impl)

**Author:** Step 00 agent · **Date:** 2026-06-12 · **Branch merged:** `or_construct/step-00`

## What I did
- Added `<initializer_list>`, `<type_traits>`, `<utility>` includes to `value_or.hpp`
  (`<iterator>` was already present).
- Added two forward declarations inside `namespace smd::free_value_or` (no defaults, matching
  existing style):
  - `template <class Ret, nullable T, class... Args, class R> constexpr R or_construct(T&&, Args&&...);`
  - `template <class Ret, nullable T, class E, class... Args, class R> constexpr R or_construct(T&&, initializer_list<E>, Args&&...);`
- Added two out-of-line definitions (outside the namespace, with `smd::free_value_or::`
  qualification), supplying both defaults (`Ret = void`, `R = conditional_t<is_void_v<Ret>, remove_cvref_t<iter_reference_t<T>>, Ret>`).
- Created `tests/beman/free_value_or/or_construct.test.cpp` — smoke test with 3 TEST_CASEs.
- Added `fvo_add_test(beman.free_value_or.tests.or_construct or_construct.test.cpp)` to
  `tests/beman/free_value_or/CMakeLists.txt` in a commented `or_construct Step 00` block.

## Build & test commands that actually worked
```bash
cmake -B build -S . -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=./infra/cmake/use-fetch-content.cmake
cmake --build build
ctest --test-dir build --output-on-failure
```
Result: **62/62 tests passed** (46 prior + 3 new `or_construct` smoke tests). Verified from
both the worktree and from the main checkout after merge.

## Toolchain / standard facts I confirmed
- g++ 15.2.0, C++23 — no changes from the inherited toolchain facts.
- The two-phase forward-decl (no defaults) + out-of-line definition (both defaults) pattern
  compiled without issue. The leading `class Ret = void` default on a function template
  preceding deduced parameters works exactly as documented.
- Overload resolution: the `{1,2,3}` braced-init-list argument in
  `fvo::or_construct(std::optional<std::vector<int>>{}, {1, 2, 3})` correctly selected the
  `initializer_list` overload — no ambiguity.
- The explicit-Ret form `fvo::or_construct<long>(std::optional<int>{7}, 0L)` correctly bound
  `7` to `Ret = long`, leaving `T` / `Args` to be deduced — `Ret` did not compete with `T`.
- The zero-arg disengaged case `fvo::or_construct(std::optional<int>{})` correctly
  default-constructs `int{}` on the disengaged path.
- Includes needed: `<initializer_list>`, `<type_traits>` (conditional_t, remove_cvref_t,
  is_void_v), `<utility>` (forward). `<iterator>` (iter_reference_t) was already present.

## Gotchas / things that bit me
- None. The design from PLAN.md §1.2 compiled exactly as written on the first attempt.
- The smoke test's braced-init-list `{1, 2, 3}` for the init-list overload: this works
  because the second parameter of the init-list overload is `std::initializer_list<E>`, which
  is what braced-init-list deduction picks. Future steps should be aware that passing a real
  `std::initializer_list<int>` object also routes to this overload.

## Issues found (if any)
- None. The §1.2 design worked without modification.

## State of `main`
- 62/62 tests pass from the main checkout (46 prior + 3 `or_construct` smoke).
- No targets excluded or skipped.
- The `fvo-step-04` leftover worktree still exists (unrelated sibling plan remnant) — ignore.

## What the next agent (Step 01) should know
- The header is now **locked** — Step 01 and all later steps must NOT edit it.
- Both overloads compile and run. Overload selection is clean: braced-init-list routes to the
  init-list overload, ordinary args to the pack overload, explicit `Ret` binds to the first
  template parameter without competing with `T`.
- Step 01 (`steps/step-01-behavior.md`) covers runtime engaged/disengaged + return-type +
  value categories over all nullable types for the single-arg pack overload. It should add a
  new `or_construct_behavior.test.cpp` (or similar) via `fvo_add_test` — do NOT add to the
  existing smoke test.
- The `fvo` namespace alias (from `test_types.hpp`) is `namespace fvo = smd::free_value_or;`.
  Always write `fvo::or_construct`.
- Negative-compile diagnostic regex: `"no matching function for call to"` (same as the
  existing `*_non_nullable_fail` tests, verified to work on g++ 15.2).
- Return type for default `Ret`: `std::remove_cvref_t<std::iter_reference_t<T>>` — for
  `optional<int>`, that's `int`; for `optional<string>`, that's `string`.
