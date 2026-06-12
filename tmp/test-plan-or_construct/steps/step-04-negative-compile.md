# Step 04 — Negative-compile: non-nullable argument rejected (both overloads)

**Goal:** prove the `nullable` constraint bites on `or_construct`, *for the right reason*,
for **both** overloads — exactly as sibling Step 04 did for `value_or` / `reference_or` /
`or_invoke`. Each case is a TU that must fail to compile, registered with the existing
`fvo_add_compile_fail_test(<name> <source> <regex>)` helper (WILL_FAIL +
`PASS_REGULAR_EXPRESSION`, matching the *intended* diagnostic — not just any error).

> There is **no dangling axis** for `or_construct` — the result is a prvalue value type and
> cannot bind to a temporary. So, unlike sibling Step 04's Family A, this step has only the
> concept-violation family (plus one optional documentation case).

> Read the sibling Step 04 handoff (`../test-plan/handoff/`, the negative-compile one) for the
> **exact GCC diagnostic string** for a failed `nullable` constraint. On this toolchain it was
> `"no matching function for call to"`. Reuse it; widen only if the two-overload candidate set
> changes the message (verify by compiling one bad TU by hand first).

## Files (one tiny TU per case, in `tests/beman/free_value_or/`)
Each: SPDX line, the one-line `fvo` alias at the top (these TUs can't use `test_types.hpp`'s
alias if they don't include it — include it, or add `namespace fvo = smd::free_value_or;`), a
comment stating *why it must not compile*, and a single `void test()` triggering exactly one
violation.

### Family — non-nullable first argument rejected by `nullable`
1. `or_construct_non_nullable_fail.cpp` (pack overload):
   `fvo::or_construct(5, 3);` — `int` is not `nullable`.
2. `or_construct_initlist_non_nullable_fail.cpp` (init-list overload):
   `fvo::or_construct(std::string{"a"}, {1, 2, 3});` — `std::string` is not `nullable`, and
   the braced-init-list forces the init-list overload, so this confirms *that* overload is
   also constrained. (Pick a first arg that is unambiguously non-nullable and whose element
   type matches a plausible init-list, so the *only* reason to reject is the constraint.)

   Regex for both: `"no matching function for call to"` (the constraint-failure family).
   Confirm the failure mentions `or_construct` / constraint, not some unrelated lookup error.

### Family — explicit `Ret` with a payload NOT convertible to it
This is the negative side of the "payload need only be *convertible* to `R`" generalization
(`PLAN.md` §1.1). The engaged path `static_cast<R>(*m)` rejects a non-convertible payload.
- `or_construct_payload_not_convertible_fail.cpp`:
  `fvo::or_construct<int>(std::optional<std::string>{}, 0);` — the held `std::string` is not
  convertible to the requested `int`.

  **Verified diagnostic on g++ 15.2:**
  `error: invalid 'static_cast' from type 'std::__cxx11::basic_string<char>' to type 'int'`.
  Regex: `"invalid .*static_cast"` (tighten/adjust to the actual wording you capture). This
  is an in-body error, *not* a constraint failure, **by design** — `PLAN.md` §1.1 resolves
  this as a Mandates-style hard error (no SFINAE constraint), because there is no alternative
  overload for a constraint to usefully redirect to, and a clear "not convertible" beats a
  vague "no matching function". Add a comment in the TU saying so, so a future reader knows
  the `static_cast` diagnostic is the *intended, stable* target, not an accident. (Only if
  LEWG later moves to a Constraint does this regex change to the constraint-failure family.)

### Optional documentation case (add only if the diagnostic is stable)
3. `or_construct_no_viable_ctor_fail.cpp`: a *nullable* first arg but arguments that cannot
   construct `R`, e.g. `fvo::or_construct(std::optional<int>{}, "not an int");` — `int` is not
   constructible from `const char*`. This failure is **inside the function body** (`R(args...)`),
   *not* a constraint failure, so the diagnostic differs (e.g.
   `"no matching .*conversion"` / `"cannot convert"` / `"no matching function for call to 'int"`).
   Capture the exact wording by compiling it by hand and write a tight regex. This documents
   *where* a bad-argument error surfaces (body, not SFINAE) — a real difference from the
   concept rejection. If the wording is fragile across compilers, mark it clearly as
   toolchain-specific in a comment, or skip it and note why in the handoff.

## CMake
Register each via `fvo_add_compile_fail_test(<name> <source.cpp> "<regex>")` in a commented
`or_construct Step 04` block. They are `EXCLUDE_FROM_ALL` (the helper handles this), so a
normal build stays green; only ctest drives them.

## Build & verify
```bash
cmake -B build -S . -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=./infra/cmake/use-fetch-content.cmake
cmake --build build                 # normal build still green (fail-TUs excluded)
ctest --test-dir build --output-on-failure   # the *_fail tests "pass" by failing-to-compile
```
Sanity-check each fail-test the way sibling Step 04 did: temporarily change the source to a
*different* error and confirm the test then FAILS (wrong reason), then restore. Note this in
the handoff; don't leave it loosened.

## Done criteria
- Both concept-violation TUs fail to compile for the regex-matched constraint reason; normal
  build unaffected; merged to `main`.
- (If included) the no-viable-ctor TU fails for its documented, distinct reason.

## Notes for your handoff
- The final regexes used (compiler-version-specific — flag for portability), and whether the
  two-overload candidate set changed the constraint diagnostic vs. the single-overload
  sibling functions.
- Whether you kept or dropped the optional no-viable-ctor case, and the exact diagnostic.
