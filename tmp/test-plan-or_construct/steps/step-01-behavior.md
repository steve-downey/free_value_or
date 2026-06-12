# Step 01 — `or_construct` behavior, return type, value categories (single-arg)

**Goal:** confirm `or_construct(m, arg)` returns `*m` (as a value) when engaged and an
`R`-constructed-from-`arg` when disengaged, with return type
`R = remove_cvref_t<iter_reference_t<T>>`, across every nullable type and value-category
combination. This is the `or_construct` analogue of sibling Step 02 (`value_or`). The
multi-arg / `initializer_list` surface is Step 02; **laziness** is Step 03 — keep those
light here.

## File
`tests/beman/free_value_or/or_construct.test.cpp` — extend the Step 00 smoke file (or add a
second file `or_construct_behavior.test.cpp` via `fvo_add_test`; either is fine — say which
in the handoff). Include `value_or.hpp` twice + `test_types.hpp`.

## Cases (sweep the nullable types holding `int`: `optional<int>`, `int*`,
`shared_ptr<int>`, `unique_ptr<int>`, `expected<int,int>`; `optional<int&>` deferred to
Step 06). Use `TEMPLATE_TEST_CASE` or `SECTION`s, matching the style of `value_or.test.cpp`.

For each nullable type holding `int`, single `int` argument:
1. **Engaged** → `or_construct(m, 99)` equals the held value (e.g. `42`), **not** the
   argument. **The result is constructed from the held value**, not aliased to it: the
   engaged path is `static_cast<R>(*m)`, which builds a fresh `R` from `*m` (a copy for the
   standard payloads; a converting construction if `*m`'s decayed type ever differs from
   `R`). Confirm it is a *copy*, not a reference, by mutating the source after the call and
   checking the result is unchanged — and contrast with `reference_or`, which would bind a
   reference. (The returned `T` can never be a reference here; `R` is a decayed value type.)
2. **Disengaged** → equals `int(99)` i.e. `99` — the result is constructed from the argument.
3. **Return type:** `static_assert(std::is_same_v<decltype(or_construct(m, 99)), int>)`.
   `R = remove_cvref_t<iter_reference_t<T>>`; for every int-payload nullable, including
   `const optional<int>`, that is a prvalue `int`. Document the computed `R` in comments, the
   way `value_or.test.cpp` does.
4. **Value categories of `m`:** lvalue, `const` lvalue, rvalue (`std::move`) — all give the
   same value result. (For move-only `unique_ptr`, engaged-rvalue is the natural case; don't
   try to copy it — see the `value_or.test.cpp` `unique_ptr` pattern.)
5. **Value categories of the argument:** lvalue arg (`int u = 99; or_construct(m, u)`) and
   rvalue/temporary arg (`or_construct(m, 99)`). Both construct the same value when
   disengaged.

## Conversion / construction-from-a-different-type
Add a case where the argument type differs from the payload but **constructs** it:
- `optional<long>` disengaged with an `int` arg → `R` is `long`, result is `long(99)`.
- `optional<int>` disengaged with a `long` arg → `R` is `int`, result is `int(99L)` (a
  *narrowing construction* — note that, unlike `value_or`'s `common_type` promotion, here the
  arg is converted **inward** to the payload type because `or_construct` constructs `R`
  directly; this is the deliberate P3413 semantics. Record the contrast with `value_or` in
  the handoff — it is exactly the difference the paper draws.)

## Explicit `Ret` — payload convertible to a *different* result type
The default cases above all have `R` = the payload type (`Ret` omitted). Now exercise the
generalization where the caller names `R` and the payload need only be **convertible** to it
(`PLAN.md` §1.1):
1. **Widen on the engaged path:** `or_construct<long>(optional<int>{7}, 0L)` → `R` is `long`,
   result `7L` (engaged: `int`→`long` conversion of `*m`). Disengaged
   `or_construct<long>(optional<int>{}, 9L)` → `9L`.
   `static_assert(std::is_same_v<decltype(or_construct<long>(m, 0L)), long>)`.
2. **Cross-type construction:** a payload type convertible to a different result type, e.g.
   `or_construct<std::string>(optional<std::string_view>{"held"}, "fb")` → engaged converts
   the held `string_view` to `std::string` (`"held"`); disengaged builds `std::string("fb")`.
3. **Explicit `Ret` with multi-arg disengaged construction:**
   `or_construct<std::string>(optional<std::string_view>{}, 3, 'x')` → `"xxx"` (engaged would
   convert the held `string_view`; disengaged uses `string(3,'x')`). Confirms the explicit-R
   form composes with emplace args (Step 02 covers emplace depth).
4. **Return type is exactly `Ret`** (not a `common_type` with the payload) — `static_assert`
   it. This is the contrast to `value_or`: there is no widening *negotiation*; the caller
   *states* the type and the payload must convert to it.

## Non-int payload sweep
Add at least one object-payload single-arg case: `optional<std::string>` disengaged with a
`std::string` arg (and a `const char*` arg → constructs a `std::string`). Engaged returns the
contained string. `R` is `std::string` (default `Ret`).

## Build & verify
C++23 configure/build/ctest. All green (existing 46 + your new assertions).

## Done criteria
- Engaged/disengaged, return-type `static_assert`s, and value-category sweeps pass for every
  nullable type.
- Merged to `main`.

## Notes for your handoff
- The `R` per type (record the table, as sibling Step 02 did).
- The **inward conversion** observation (`optional<int>` + `long` arg → `int`) vs `value_or`'s
  outward `common_type` promotion — Step 06's README contrast depends on it.
