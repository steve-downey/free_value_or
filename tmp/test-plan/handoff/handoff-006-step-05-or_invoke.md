# Handoff 006 — after Step 05 (or_invoke results + laziness)

**Author:** Step 05 agent · **Date:** 2026-06-12 · **Branch merged:** `test/step-05`

## What I did

Added `tests/beman/free_value_or/or_invoke.test.cpp` (new exe via `fvo_add_test`) and a
5-line CMakeLists.txt entry for Step 05.

### Coverage in or_invoke.test.cpp

1. **Return-type static_asserts** — 3 cases at file scope:
   - `optional<int>` + `int(*)()` → `int`
   - `optional<int>` + `long(*)()` → `long`
   - `optional<int>` + `int&(*)()` → `int` (reference-returning invocable decays via `common_type`)

2. **Runtime engaged/disengaged** — 5 nullable types: `optional`, `expected`, `int*`, `shared_ptr`, `unique_ptr`

3. **Laziness (headline)** — 3 test cases:
   - Engaged: `call_count == 0` after call
   - Disengaged: `call_count == 1` (exactly once)
   - Contrast with `value_or` eagerness (both counts increment for `value_or`)

4. **Value categories of m** — lvalue, const lvalue, rvalue (each with laziness verification)

5. **Type promotion** — invocable returning `long`, result is `long`

6. **Reference-returning invocable** — `invoke_result_t<I> = int&`, `common_type_t<int&, int&> = int` (value copy, not reference)

7. **Move-only invocable** — `unique_ptr`-capturing lambda passed as `std::move(f)`:
   - Engaged path: invocable NOT called, `unique_ptr` stays valid
   - Disengaged path: invocable called, dereferences `unique_ptr`

8. **Mutable lambda** — non-const `operator()` works via `I&&` forwarding

## Build & test commands that actually worked

```bash
cmake -B build -S . -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=./infra/cmake/use-fetch-content.cmake
cmake --build build
ctest --test-dir build --output-on-failure
```

Result: **45/45 tests pass** (14 new or_invoke tests + 31 existing).

## Toolchain / standard facts I confirmed

No new surprises. Everything from prior handoffs still holds:
- GCC 15.2.0, C++23, libstdc++ `__cpp_lib_optional = 202110`
- `common_type_t<int&, int&>` = `int` — stripes references (standard behaviour, not a quirk)
- `invoke_result_t<I>` (not `invoke_result_t<I&&>`) used in the default `R` — passes through OK because `invocable()` inside the template body calls on the rvalue-ref-bound `I&&`

## Gotchas / things that bit me

- The static_assert for function-pointer types uses `int(*)()` etc. rather than a lambda type,
  since you can't `std::declval` a generic lambda. This is fine and clear.
- `fvo_add_test` already includes `${CMAKE_CURRENT_SOURCE_DIR}` and links `beman::optional`,
  so no extra CMake plumbing needed.

## Key finding: or_invoke vs reference_or asymmetry

`or_invoke` uses `common_type_t` (not `common_reference_t`), so even a reference-returning
invocable yields a **value** result, not a reference. `reference_or` uses `common_reference_t`
and can yield a reference. Step 07/finalize should note this asymmetry in the test README.

## Issues found

None. No header bugs.

## State of `main`

45/45 tests pass from the main checkout after the merge. All prior tests green.

## What the next agent (Step 06) should know

- **Step 06** is `constexpr` constant-evaluation tests — `steps/step-06-constexpr.md`.
- Use `fvo_add_test(name source)` for the new test exe.
- All three functions (`value_or`, `reference_or`, `or_invoke`) have `constexpr` on their
  declarations — confirm `consteval` / `static_assert` driven tests work at C++23.
- The vendored `beman::optional` may or may not be `constexpr`-friendly for `optional<T&>`;
  check before including it in constexpr tests (or gate with `#if FVO_HAS_OPTIONAL_REF`).
