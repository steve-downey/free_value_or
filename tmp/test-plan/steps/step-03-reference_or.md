# Step 03 — `reference_or` reference semantics

**Goal:** confirm `reference_or(m, u)` yields an actual *reference* to the held object when
engaged and to `u` when disengaged, with `R = common_reference_t<iter_reference_t<T>, U&&>`,
and that no copy happens on the engaged path. (Dangling *rejection* is Step 04; here we test
the well-formed, non-dangling cases.)

## File
`tests/beman/free_value_or/reference_or.test.cpp` (new exe via Step 00 helper, C++23).

## Cases (sweep the confirmed nullable types holding `int`, plus a `std::string` payload)
1. **Engaged returns a reference to the contained object.** Verify *identity*:
   `&reference_or(m, fallback) == &(*m)` (take address of the engaged storage). Use an
   lvalue `m` and lvalue `fallback` of a compatible lvalue type so `R` is an lvalue
   reference (e.g. `int&`). This proves no copy.
2. **Disengaged returns a reference to `fallback`.** `&reference_or(m_empty, fallback) ==
   &fallback`.
3. **Return type:** `static_assert(std::is_reference_v<decltype(reference_or(m, u))>)` for
   the lvalue/lvalue case, and assert the exact `R` (e.g. `int&` / `const int&`) per the
   `common_reference` rule. Contrast with Step 02's `common_type` result in a comment.
4. **const propagation:** `reference_or(const optional<int>& , const int&)` should yield
   `const int&`; mutating through it must be ill-formed (you can note this; the hard
   ill-formed check belongs to Step 04 only if you want belt-and-suspenders).
5. **Compatible-but-not-identical types that are still safe references:** e.g.
   `optional<int>` engaged with an `int` lvalue fallback (both `int&` → `R = int&`). Avoid
   cases that would create a temporary `R` — those are Step 04's negative tests; if you
   accidentally write one, the header's `static_assert` will stop you (good signal).
6. **Mutation round-trip:** with non-const `int& r = reference_or(m, fb);` on the engaged
   path, writing `r = 99` changes `*m`; on the disengaged path it changes `fb`. Confirms a
   true reference, not a copy.

## Important
`reference_or` only compiles for argument combinations where `R` does **not** bind to a
temporary (the two `reference_constructs_from_temporary_v` static_asserts). So every case
here must be a genuine, non-dangling reference scenario. If you find a *safe* case that the
static_assert nonetheless rejects (false positive), that's a header finding — record it,
don't fix the header.

## Build & verify
C++23 configure/build/ctest. All green.

## Done criteria
- Reference identity, return-type, const-propagation, and mutation tests pass.
- Merged to `main`.

## Notes for your handoff
- The list of `(T, U)` combinations you confirmed are *safe references* vs. which you found
  *must* be temporaries — Step 04 turns the latter into negative-compile tests, so a
  concrete list saves that agent time.
