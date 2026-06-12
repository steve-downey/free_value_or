# Step 07 — `std::optional<T&>` (C++26) + finalize

**Goal:** (A) exercise the C++26 `optional<T&>` paths that earlier steps gated out; (B) do a
final consolidation pass: full-suite run, a short test-suite README, and a clean closing
handoff.

## Part A — `optional<T&>` (only if the toolchain supports it)
From Step 00's handoff: confirm whether `std::optional<int&>` compiles at `-std=c++26` on
this libstdc++. If **not** supported, skip Part A, document clearly that it's blocked on
toolchain, and leave the gated sections in place for a future run. If supported:

1. **C++26 test target.** Enable the C++26 test executable mechanism Step 00 added (the
   `BEMAN_FREE_VALUE_OR_TESTS_CXX26` option / second helper). Build the relevant
   `optional<T&>`-bearing TUs at `-std=c++26`.
2. **File** `tests/beman/free_value_or/optional_ref.test.cpp` covering, for
   `std::optional<int&>`:
   - `nullable<std::optional<int&>>` holds (a `static_assert`, complementing Step 01).
   - **`value_or`**: engaged returns the referent's value; disengaged returns fallback.
     Note `iter_reference_t<optional<int&>>` is `int&`, so reason about `R` and document it.
   - **`reference_or`**: engaged returns a reference *to the referent* — verify identity
     `&reference_or(o, fb) == &referent`. This is the key `optional<T&>` use case: the
     reference threads through. Disengaged → reference to `fb`.
   - **`or_invoke`**: engaged returns referent value; disengaged calls `f` once (reuse the
     call-count laziness check).
   - Rebinding semantics: change what the `optional<int&>` refers to and confirm the
     functions follow the new referent.
3. Wire it through `catch_discover_tests` under the C++26 target. Keep the default C++23
   build green (the C++23 build simply compiles these sections out via `FVO_HAS_OPTIONAL_REF`).

## Part B — finalize
4. **Full-suite run** from the main checkout after merge:
   ```bash
   cmake -B build -S . -DCMAKE_CXX_STANDARD=23 \
     -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=./infra/cmake/use-fetch-content.cmake
   cmake --build build
   ctest --test-dir build --output-on-failure
   # plus the C++26 target run if enabled
   ```
   Confirm: all positive tests pass; all `*_fail` negative tests pass (fail-to-compile for
   the right reason). Record total counts.
5. **`tests/beman/free_value_or/README.md`** (short): what each test file covers, the
   standard-version requirements (C++23 baseline, C++26 for `optional<T&>`), how to run the
   suite and the negative-compile tests, and a one-line note on the `value_or`/`or_invoke`
   (`common_type`, value) vs `reference_or` (`common_reference`, reference) asymmetry.
6. **CHECKLIST.md:** tick Step 07; move any open items into the "Issues" section with a
   clear status (resolved / deferred / blocked-on-toolchain).
7. **Closing handoff** `handoff-NNN-final.md`: summary of the whole suite, coverage map vs.
   PLAN §3 matrix (what's covered, what's deferred and why), and any header findings the
   human owner should review.

## Done criteria
- `optional<T&>` covered (or explicitly documented as toolchain-blocked).
- Full suite (positive + negative, C++23 and, if available, C++26) green from `main`.
- Test README written; CHECKLIST and final handoff complete.
- Merged to `main`.

## Notes
- This is the consolidation step — if earlier steps left small gaps (a missing
  value-category, a TODO), close them here or log them explicitly. Don't leave silent gaps.
