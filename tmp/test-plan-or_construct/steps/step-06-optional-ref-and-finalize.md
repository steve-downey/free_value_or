# Step 06 — `optional<T&>` (C++26) coverage + finalize / consolidate

**Goal:** complete the matrix with the C++26 reference-optional nullable, then consolidate:
one clean full-suite run, and a test README update documenting `or_construct`. Mirrors
sibling Step 07.

## Part A — `optional<T&>` via `beman::optional`

`std::optional<T&>` (P2988) is not in the local libstdc++; the suite vendors
`beman::optional` and exposes it as `fvo_opt::optional<T&>` when `FVO_HAS_OPTIONAL_REF=1`
(injected by `fvo_add_test`; see `test_types.hpp`). Add `or_construct` coverage for it,
feature-gated exactly like the existing `optional_ref.test.cpp`:

```cpp
#if FVO_HAS_OPTIONAL_REF
// ... sections using fvo_opt::optional<T&> ...
#endif
```

File: extend `or_construct.test.cpp` with a gated section, or add
`or_construct_optional_ref.test.cpp` (new exe via `fvo_add_test`). Cases for an
`fvo_opt::optional<int&>`:
1. **Engaged** → `or_construct(opt_ref, 99)` returns the referred-to value (e.g. `7`).
   **Return type** (default `Ret`) is `R = remove_cvref_t<iter_reference_t<optional<int&>>>`.
   Work out what `iter_reference_t` is for `optional<int&>` (dereferencing yields `int&`, so
   `R` is `int`) and `static_assert` it. Record the computed `R` — this is the one type where
   it's worth double-checking, because the payload is itself a reference. Add one explicit-`Ret`
   case too — `or_construct<long>(opt_ref, 0L)` → `long` (the held `int&` converts to `long`).
2. **Disengaged** → `or_construct(opt_ref_empty, 5)` constructs `int(5)`. Result is a value
   `int`, **not** a reference (this function always returns the decayed value type — contrast
   `reference_or`, which returns a reference). Make that contrast explicit in a comment.
3. One **init-list / multi-arg** case if the payload type makes it meaningful (e.g.
   `optional<std::string&>` disengaged with `{'a','b','c'}` → a `std::string` value). Note
   the result is a fresh value, not a reference to anything.
4. Value categories of the `optional<T&>` as in earlier steps.

## Part B — finalize / consolidate

1. **Full clean run** from the main checkout:
   ```bash
   rm -rf build
   cmake -B build -S . -DCMAKE_CXX_STANDARD=23 \
     -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=./infra/cmake/use-fetch-content.cmake
   cmake --build build
   ctest --test-dir build --output-on-failure
   ```
   All positive tests green; all negative (`*_fail`) tests "pass" by failing-to-compile with
   their regex. Record the final total count (existing + new).
2. **README update:** add an `or_construct` section to
   `tests/beman/free_value_or/README.md` (the file the sibling suite maintains), covering:
   - what `or_construct` is and the two overloads, each with a leading `Ret` parameter;
   - result type: `Ret` defaulted → decayed payload type; `Ret` explicit → that type, with
     the payload required only to be **convertible** to it (**no** `common_type` negotiation,
     unlike `value_or`);
   - **laziness** (fallback constructed only when disengaged) — the headline;
   - the **inward construction** contrast vs `value_or`'s outward `common_type` promotion
     (from Step 01's handoff);
   - that it **cannot dangle** (prvalue result), so there is no dangling negative-compile
     family, unlike `reference_or`;
   - the matrix coverage (types × value categories × construction arities × init-list ×
     constexpr × optional<T&>).
3. **Confirm the existing 46 sibling tests are untouched and still pass**, and that no header
   change beyond Step 00's two overloads was made.

## Done criteria
- `optional<T&>` `or_construct` cases pass under `FVO_HAS_OPTIONAL_REF=1`.
- Full suite green from a clean build; README documents `or_construct`.
- Merged to `main`. `CHECKLIST.md` fully ticked.

## Notes for your handoff (final)
- Final total test count.
- The `optional<T&>` computed `R` (confirm it's the value type `int`, not `int&`).
- A one-paragraph wrap-up: the gap is closed — `or_construct` exists, is tested across the
  same matrix as the main APIs, and the D4270 paper can now (separate follow-up) update its
  "Relation to P3413R0" comparison table to replace "via lambda; no direct analogue" with a
  pointer to `or_construct`.
