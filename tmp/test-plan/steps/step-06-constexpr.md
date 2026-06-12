# Step 06 — `constexpr` / constant-evaluation coverage

**Goal:** all three functions are declared `constexpr`; confirm they actually work in a
constant-evaluated context (not merely that they compile at runtime).

## File
`tests/beman/free_value_or/constexpr.test.cpp` (new exe via Step 00 helper, C++23).

## Approach
The strongest form is `static_assert(... == expected);` evaluated at compile time — that
*forces* constant evaluation. Use `constexpr` variables / `consteval`-style helpers.

## Cases
1. `value_or`:
   - `static_assert(value_or(std::optional<int>{7}, 0) == 7);` (engaged)
   - `static_assert(value_or(std::optional<int>{}, 5) == 5);` (disengaged)
   - A raw-pointer case in constant context: pointer to a `constexpr` static/`constinit`
     object, plus the `nullptr` disengaged case. (Raw pointers are usable in constant
     expressions when pointing at a permitted constexpr object — verify what the toolchain
     allows; if pointer-in-constexpr is too fiddly, use `optional` and note it.)
2. `reference_or`:
   - A constant-evaluated case that yields a reference to a `constexpr` object and reads
     through it. e.g. `static constexpr int v = 3; static_assert(reference_or(
     std::optional<int>{...}, v) == v_or_held);` — pick engaged & disengaged so the asserted
     value distinguishes the branches. Keep it non-dangling (Step 04 covers dangling).
3. `or_invoke`:
   - `static_assert(or_invoke(std::optional<int>{9}, []{ return 0; }) == 9);` (engaged; the
     lambda is a `constexpr`-callable.)
   - `static_assert(or_invoke(std::optional<int>{}, []{ return 4; }) == 4);` (disengaged)
   - Confirm the invocable used in constant context is itself `constexpr`-eligible.

Add a trivial `TEST_CASE` (e.g. `SUCCEED()`) so ctest has something to run; the real proof
is the `static_assert`s compiling.

## Notes / gotchas
- `std::optional`'s relevant ops are `constexpr` in C++23 — fine. `std::unique_ptr` /
  `std::shared_ptr` are **not** usable in constant expressions; **do not** try to constexpr
  them. Stick to `optional` and (carefully) raw pointers.
- `std::expected` ops are `constexpr` in C++23 — you may add one expected case.

## Build & verify
C++23 configure/build/ctest. The TU must compile (static_asserts hold).

## Done criteria
- constexpr static_asserts for all three functions (engaged + disengaged) compile.
- Merged to `main`.

## Notes for your handoff
- Which types you could and couldn't use in constant context on this toolchain.
