# Step 01 — `nullable` concept coverage

**Goal:** lock down exactly which types satisfy `smd::free_value_or::nullable` and which do
not, entirely at compile time. Pure `static_assert`s in one Catch2 TU (the TU still needs a
`TEST_CASE` so the exe links and ctest sees it, but the real testing is the static_asserts —
this mirrors `~/src/transcode/main/.../concepts.test.cpp`).

## File
`tests/beman/free_value_or/concept.test.cpp` (add a test exe via the Step 00 helper).
Include `<beman/free_value_or/value_or.hpp>` **twice** (include-guard idempotency check,
matching the existing convention) and `test_types.hpp`.

## Positive assertions — `static_assert(nullable<X>)`
- `std::optional<int>`, `std::optional<std::string>`
- `std::expected<int, int>` — confirm it really satisfies the concept; if it does NOT (e.g.
  because `bool(t)`/`*t` aren't both valid on a `const` expected the way the concept needs),
  record that as a finding, not a failure to force.
- `int*`, `const int*`
- `std::shared_ptr<int>`, `std::unique_ptr<int>`
- `std::optional<int&>` — **gated** behind `FVO_HAS_OPTIONAL_REF` from `test_types.hpp`.
- Sanity on cv/ref forms the functions actually take: since the functions take `T&&` and
  the concept is on the bare `T`, assert the bare-type forms. Also spot-check
  `nullable<const std::optional<int>>` to document behavior (concept already uses `const T`
  internally).

## Negative assertions — `static_assert(!nullable<X>)`
- `int`, `double` (contextually bool, but no `operator*`)
- `std::string`, `std::vector<int>`
- `bool_only` (operator bool, no deref)
- `deref_only` (deref, no operator bool)
- `nonconst_nullable` (members non-const → fails the `const T` requirement). **If this one
  surprises you** (e.g. it still satisfies the concept), document why in the handoff — it's
  a subtle point about how the `requires(const T t)` parameter interacts with conversions.
- `void`, `std::nullptr_t` (document actual result; `nullptr_t` has no `operator*`).

## Build & verify
Same C++23 configure/build as Step 00 (copy the exact commands from the latest handoff).
The test "passes" by compiling; the `TEST_CASE` should `SUCCEED()` or check one trivial
runtime fact so ctest is green.

## Done criteria
- `concept.test.cpp` compiles (all static_asserts hold) and the exe runs green.
- Any concept behavior that differed from the matrix in PLAN §3 is written up in the handoff
  and CHECKLIST "Notes", so Steps 02–05 don't trip over it (e.g. "expected is/ isn't
  nullable", "nonconst type behavior").
- Merged to `main`.

## Notes for your handoff
- The definitive list of what is and isn't `nullable` on this toolchain. Later steps pick
  their runtime cases from your *positive* list — make it unambiguous.
