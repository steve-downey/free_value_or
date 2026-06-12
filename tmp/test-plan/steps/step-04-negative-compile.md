# Step 04 — Negative-compile tests (things that must NOT compile)

**Goal:** prove the constraints bite, *for the right reason*. Two families:
(A) dangling-reference rejection in `reference_or`; (B) non-nullable first argument rejected
by the `nullable` constraint in all three functions. Each is a TU that must fail to compile,
registered with the `fvo_add_compile_fail_test` helper from Step 00 (WILL_FAIL +
`PASS_REGULAR_EXPRESSION` matching the intended diagnostic — not just *any* error).

> Read the Step 00 / latest handoff for the **exact GCC diagnostic strings** for (a) a
> failed `nullable` constraint and (b) a failed `reference_constructs_from_temporary_v`
> static_assert. Use those to write tight regexes. If they weren't captured, capture them
> now by compiling one bad TU by hand first.

## Files (one tiny TU per case, in `tests/beman/free_value_or/`)
Each: SPDX line, a comment block stating *why it must not compile*, a single `void test()`
that triggers exactly one violation. Keep them minimal so only the intended error fires.

### Family A — dangling rejection (`reference_or`)
Construct cases where `R = common_reference_t<iter_reference_t<T>, U&&>` would bind to a
temporary, tripping `static_assert(!reference_constructs_from_temporary_v<R, U>)` or
`<R, T&>`. Candidate cases (verify each truly fails — pick the ones that do on this
toolchain; use Step 03's handoff list):
- `ref_or_temp_from_literal_fail.cpp`: `std::optional<std::string> o; reference_or(o,
  "literal");` — `R` becomes a `std::string` temporary bound to a reference.
- `ref_or_temp_from_conversion_fail.cpp`: `std::optional<int> o; long n = 1;
  reference_or(o, n);` — if `common_reference<int&, long&>` is a prvalue, the engaged `*o`
  (`int&`) must convert to a temporary → rejected. (If this *compiles* on your toolchain,
  it means `common_reference` found a safe type; drop it and note why.)
- `ref_or_rvalue_default_fail.cpp`: a case binding to an rvalue fallback that would dangle,
  e.g. `reference_or(o, std::string{"x"})` where `R` is `std::string&`.

  For each, the regex should match the `reference_constructs_from_temporary` static_assert
  text (capture exact wording from the toolchain).

### Family B — non-nullable argument rejected by `nullable`
One file per function so a regression localizes:
- `value_or_non_nullable_fail.cpp`: `value_or(5, 3);` (`int` is not `nullable`).
- `reference_or_non_nullable_fail.cpp`: `reference_or(std::string{"a"}, std::string{"b"});`
- `or_invoke_non_nullable_fail.cpp`: `or_invoke(std::vector<int>{}, []{ return 0; });`

  Regex: the GCC "constraint not satisfied" / "no matching function ... constraints not
  satisfied" wording (capture exact). The point is to confirm rejection is due to
  `nullable`, not some unrelated overload/lookup error.

### Optional extra (if cheap)
- `or_invoke_bad_invocable_fail.cpp`: pass a non-invocable as the second arg — documents
  that `invoke_result_t<I>` failure is the cause. Only add if the diagnostic is stable.

## CMake
Register each via `fvo_add_compile_fail_test(<name> <source.cpp> "<regex>")`. They must be
`EXCLUDE_FROM_ALL` so a normal build stays green; only ctest drives them.

## Build & verify
```bash
cmake -B build -S . -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=./infra/cmake/use-fetch-content.cmake
cmake --build build               # normal build still green (fail-TUs excluded)
ctest --test-dir build --output-on-failure   # the *_fail tests "pass" by failing-to-compile w/ matching regex
```
Verify each fail-test actually exercises its regex: temporarily loosen the source to a
*different* error and confirm the test then FAILS (wrong reason) — then restore. Note this
sanity step in the handoff; don't leave it loosened.

## Done criteria
- Every negative test passes (fails to compile for the intended, regex-matched reason).
- Normal build unaffected. Merged to `main`.

## Notes for your handoff
- Which candidate dangling cases actually failed vs. compiled-safely on this toolchain (a
  case that compiles is itself a documented finding about `common_reference`).
- The final regexes used (compiler-version-specific — flag that for portability).
