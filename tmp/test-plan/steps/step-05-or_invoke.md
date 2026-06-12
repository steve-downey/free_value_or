# Step 05 — `or_invoke` results and laziness

**Goal:** confirm `or_invoke(m, f)` returns `*m` when engaged and `f()` when disengaged, with
return type `common_type_t<iter_reference_t<T>, invoke_result_t<I>>`, and — the headline —
that `f` is **called only when disengaged** (lazy).

## File
`tests/beman/free_value_or/or_invoke.test.cpp` (new exe via Step 00 helper, C++23).

## Cases (sweep confirmed nullable types holding `int`)
1. **Engaged** → returns `*m`; **disengaged** → returns `f()`'s result.
2. **Laziness (critical):** use an invocable with an observable side effect — a captured
   `int call_count` incremented per call, or a lambda that sets a flag.
   - Engaged: `call_count == 0` after the call (invocable NOT invoked).
   - Disengaged: `call_count == 1` (invoked exactly once).
3. **Return type:** `static_assert` the computed `R`. e.g. `optional<int>` + `[]{return
   0;}` → `common_type_t<int&, int>` → `int`. Try an invocable returning a different type
   (`[]{ return 0L; }` → `long`) and confirm `R` and the value.
4. **Invocable returning a reference:** e.g. `[&]() -> int& { return some_static; }`.
   Determine the resulting `R` (note `or_invoke` uses `common_type`, not `common_reference`,
   so a reference invocable result likely decays to a value — verify and document the
   contrast with `reference_or`).
5. **Value categories of `m`:** lvalue / const / rvalue (move-only `unique_ptr` engaged via
   rvalue). Disengaged path still calls `f` exactly once.
6. **Stateful / move-only invocable:** confirm `I&&` forwarding works (e.g. a mutable
   lambda, or one capturing a `unique_ptr`). Engaged path must not require invoking it.

## Build & verify
C++23 configure/build/ctest. All green. The laziness checks are the ones most likely to
catch a real regression — make them unmistakable.

## Done criteria
- Result, return-type, and laziness (call-count == 0 engaged / == 1 disengaged) tests pass.
- Merged to `main`.

## Notes for your handoff
- The `R` you computed for the reference-returning-invocable case (value vs reference), so
  Step 07/finalize can note the `or_invoke` vs `reference_or` asymmetry in the test README.
