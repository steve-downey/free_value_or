# Step 02 — `value_or` behavior, return type, value categories

**Goal:** confirm `value_or(m, u)` returns `*m` when engaged and `u` when disengaged, with
the right (decayed) return type, across every nullable type and value-category combination.

## File
Replace the placeholder `tests/beman/free_value_or/value_or.test.cpp` body (keep the file
and its existing target `beman.free_value_or.tests.value_or`, but bump that target to C++23
via the Step 00 helper/feature). Include `value_or.hpp` twice + `test_types.hpp`.

## Cases (use Catch2 `TEMPLATE_TEST_CASE` or `SECTION`s to sweep the nullable types from
Step 01's confirmed positive list: `optional<int>`, `int*`, `shared_ptr<int>`,
`unique_ptr<int>`, and `expected<int,int>` if Step 01 confirmed it; `optional<int&>` is
deferred to Step 07).

For each nullable type holding `int`:
1. **Engaged** → `value_or(m, fallback)` equals the held value, not the fallback.
2. **Disengaged** → equals the fallback.
3. **Return type:** `static_assert(std::is_same_v<decltype(value_or(m, u)), R>)` where `R`
   is what `common_type_t<iter_reference_t<T>, U&&>` decays to (work it out per type; e.g.
   for `optional<int>` + `int` fallback it should be a prvalue `int`). Document the
   computed `R` in comments.
4. **Value categories of `m`:** lvalue, `const` lvalue, and rvalue (`std::move`) — all give
   the same value result. (For move-only `unique_ptr`, engaged-rvalue is the natural case;
   don't try to copy it.)
5. **Value categories of `u`:** lvalue fallback and rvalue/temporary fallback.
6. **Type mismatch that still works:** e.g. `optional<int>` with a `long` or `double`
   fallback — confirm `common_type` picks the expected promoted type and the value is
   correct in both engaged/disengaged branches.

Add at least one **non-int payload** sweep (e.g. `optional<std::string>` with a
`std::string` / string-literal fallback) to catch object (non-trivial) handling — engaged
returns the contained string, disengaged returns the fallback. (Beware: a string-literal
fallback through `common_type` constructs a `std::string` value — that's fine for
`value_or`; the dangling concern is a `reference_or` issue, Steps 03/04.)

## Eagerness note
`value_or` evaluates `u` unconditionally (it's a by-value/by-ref parameter, not lazy). You
may add a small check that a fallback with observable construction is built even when
engaged — but the *lazy* contract belongs to `or_invoke` (Step 05), so keep this light.

## Build & verify
C++23 configure/build/ctest (copy commands from latest handoff). All green.

## Done criteria
- All sweeps pass; return-type static_asserts hold.
- The placeholder TODO check is gone; `value_or.test.cpp` now contains real tests.
- Merged to `main`.

## Notes for your handoff
- Any `common_type` result that surprised you (record the computed `R` per type) — Step 03
  will compare against `common_reference` and needs your `value_or` baseline.
