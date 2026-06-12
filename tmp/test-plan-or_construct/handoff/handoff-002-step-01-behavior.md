# Handoff 002 — after Step 01 (behavior)

**Author:** Step 01 agent · **Date:** 2026-06-12 · **Branch merged:** `or_construct/step-01`

## What I did
- Added `tests/beman/free_value_or/or_construct_behavior.test.cpp` — new file, NOT modifying
  the smoke test. Registered via `fvo_add_test(beman.free_value_or.tests.or_construct_behavior
  or_construct_behavior.test.cpp)` appended to `tests/beman/free_value_or/CMakeLists.txt`.
- Header was NOT modified (locked after Step 00).

## What the test file covers
- **Return-type static_asserts** for all nullable types holding `int`: `optional<int>`,
  `expected<int,int>`, `int*`, `shared_ptr<int>`, `unique_ptr<int>`, plus `const optional<int>`.
  Confirmed `R = remove_cvref_t<iter_reference_t<T>> = int` in all cases.
- **Inward-conversion static_asserts:** `optional<int> + long → R = int` (not long);
  `optional<long> + int → R = long`.  Documents the contrast with `value_or`'s `common_type`
  promotion.
- **Explicit Ret static_asserts:** `or_construct<long>(optional<int>,...) → long`;
  `or_construct<string>(optional<string_view>,...) → string`.
- **Runtime engaged/disengaged** for every nullable type (`optional`, `expected`, `int*`,
  `shared_ptr`, `unique_ptr` move-only pattern).
- **Copy semantics:** mutate source after engaged call → result unchanged (proves
  `static_cast<R>(*m)` builds a fresh value, not an alias).
- **Value categories of m:** lvalue, `const` lvalue, rvalue — all sections.
- **Value categories of argument:** lvalue and rvalue/temporary.
- **Inward conversion runtime** (4 SECTIONs): `optional<int> + long`, `optional<long> + int`,
  both engaged and disengaged.
- **Explicit Ret runtime:** widen `int → long`; `string_view → string`; multi-arg disengaged
  `string(3,'x')`; static_assert return is exactly `Ret`.
- **Non-int payload** (`std::string`): `std::string` arg and `const char*` arg.

## Build & test commands that actually worked
```bash
cd /home/sdowney/src/free_value_or/oc-step-01
cmake -B build -S . -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=./infra/cmake/use-fetch-content.cmake
cmake --build build
ctest --test-dir build --output-on-failure
```
Result: **76/76 tests passed** (62 prior + 14 new `or_construct_behavior` tests). Also
verified from the main checkout after merge: 76/76 green.

## Toolchain / standard facts I confirmed
- g++ 15.2.0, C++23 — no changes from the inherited toolchain facts.
- `remove_cvref_t<iter_reference_t<T>>` compiles and evaluates as documented for all five
  nullable types. The `const optional<int>` case: `iter_reference_t` using a `T& = const
  optional<int>&` yields `const int&`; `remove_cvref_t` strips both ref and const → `int`.
- `static_assert(std::is_same_v<decltype(fvo::or_construct(...)), int>)` at namespace scope
  (outside any test case) compiles cleanly.
- Explicit `Ret` template argument deduction: `or_construct<long>(optional<int>{7}, 0L)` — `Ret`
  binds to `long`, `T` deduces to `optional<int>`, `Args` deduces to `long`. Clean.
- `or_construct<std::string>(optional<std::string_view>{}, 3, 'x')` → `string(3,'x')` on the
  disengaged path: the variadic pack forwards correctly to the `R(args...)` constructor call.

## Gotchas / things that bit me
- None. All test patterns matched step expectations on first compilation.
- The `unique_ptr` engaged case uses `NullableFixture<int>::uptr_engaged(42)` passed as an
  rvalue — same pattern as `value_or.test.cpp`. Do not attempt to copy a `unique_ptr`.
- Multi-arg `string(3,'x')` with explicit Ret works because the `or_construct` pack overload
  is `R(std::forward<Args>(args)...)` — the arg types deduce from the call, so `int` and
  `char` are forwarded and `string(size_t, char)` picks up correctly (no narrowing issue
  with `int` → `size_t` in practice on g++).

## Issues found (if any)
- None. Header is correct; no design flaws surfaced. Did NOT modify the header.

## State of `main`
- **76/76** tests pass from the main checkout (62 prior + 14 new).
- No targets excluded or skipped.

## Key design observation to carry forward (Step 06 README)
`or_construct` uses *inward conversion*: `R = payload type`, arg converts **to** `R`.
`value_or` uses *outward promotion*: `R = common_type(payload, arg)`, both types widen to a
common type. This is the deliberate P3413 distinction: `or_construct` guarantees the return
type is always the payload type (or the caller-stated `Ret`); it never silently promotes.
Example: `value_or(optional<int>{}, 99L) → long`; `or_construct(optional<int>{}, 99L) → int`.

## What the next agent (Step 02) should know
- Step 02 (`steps/step-02-emplace-and-initlist.md`) covers zero-arg/default-construction,
  multi-arg emplace, and the `initializer_list` overload. The smoke test already has a
  `vector<int>` init-list case; Step 02 should add a new file (or extend the smoke), but must
  NOT modify `or_construct_behavior.test.cpp`.
- The `or_construct<std::string>(optional<string_view>{}, 3, 'x')` multi-arg case I added in
  Step 01 is a preview of the emplace surface — Step 02 will go deeper (more types, edge
  cases, braced init-list vs pack, zero-arg default-construction).
- Braced init-list routing to the `initializer_list` overload was confirmed in the smoke test
  (Step 00): `{1, 2, 3}` → `initializer_list<int>` overload. Step 02 should lean on this.
- `fvo_add_test` + the build command above are the only toolchain commands needed.
- Baseline: 76/76 tests pass.
