# Handoff 000 — initial (before Step 00)

**Author:** plan author · **Date:** 2026-06-12 · **Branch merged:** _(none yet)_

## What this is

The kickoff handoff for the **`or_construct`** sub-plan — adding the free-function analogue
of P3413R0's member `value_or_construct`, the one gap D4270R0 (`papers/free_value_or.tex`,
"Relation to P3413R0") explicitly leaves open. This is a *sibling* of the existing
`tmp/test-plan/` suite (which covers `value_or` / `reference_or` / `or_invoke`, steps 00–07,
46 tests green through its Step 06).

Read `PLAN.md` (the gap, the design, the matrix) then `steps/step-00-impl.md`. Your step is
**Step 00**.

## State of the repo right now

- Main checkout: `/home/sdowney/src/free_value_or/main` (bare repo lives at
  `…/free_value_or/free_value_or.git`; the old sibling prompt's `…/free_value_or/free_value_or`
  path is **stale**).
- A leftover **`fvo-step-04`** worktree/branch from the sibling suite exists
  (`git worktree list`). It is unrelated to this plan — **do not reuse it**.
- The header `include/beman/free_value_or/value_or.hpp` currently declares exactly three
  functions (`reference_or`, `value_or`, `or_invoke`) in `namespace smd::free_value_or`,
  two-phase (forward-declared in-namespace, defined out-of-line). `or_construct` does **not**
  exist — Step 00 adds it.
- The sibling suite's Step 07 (`optional<T&>` finalize) is still open in *its* checklist; it
  does not block this plan. `beman::optional` is already vendored and wired
  (`FVO_HAS_OPTIONAL_REF=1`).

## Design decided (see PLAN.md §1 for full rationale)

- Name: **`or_construct`** (D4270's own suggested spelling; not `value_or_construct`, which is
  P3413's *member*).
- Two overloads: forwarded **arg pack**, and **`std::initializer_list<E>` + trailing args**.
  Each carries a **leading `class Ret = void`** template parameter.
- Result type: `Ret` omitted → `R = std::remove_cvref_t<std::iter_reference_t<T>>` (decayed
  payload type, P3413 parity); `Ret` given (`or_construct<R>(…)`) → `R = Ret`, and the
  payload `U` need only be **convertible to** `R` (engaged path `static_cast<R>(*m)`), not
  equal to it. No `common_type` (there's an arg pack, no single second type). Result is
  always a prvalue → cannot dangle, so **no** `reference_constructs_from_temporary_v` guards.
- **Verified on g++ 15.2 (`-std=c++23`):** both the one-overload-each `Ret=void` sentinel
  design and the two-phase decl/def split (defaults on the definition) compile and behave;
  the explicit-`Ret` widening/cross-type cases run; the non-convertible-payload case fails
  with `invalid 'static_cast' … to type 'int'`; non-nullable fails on the `nullable`
  constraint. See PLAN.md §1.1–§1.2.
- **Lazy** like `or_invoke`: the fallback is constructed only on the disengaged path. This is
  the headline behavior (Step 03).
- Engaged path observes (`static_cast<R>(*m)`), does not consume — same rule as `value_or`.

## Build / test (verified by the sibling suite, unchanged here)

```bash
cmake -B build -S . -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=./infra/cmake/use-fetch-content.cmake
cmake --build build
ctest --test-dir build --output-on-failure
```
Baseline: 46/46 tests pass before any `or_construct` work. Keep them green.

## What Step 00 should know

- Step 00 is the **only** step permitted to edit the header. Add exactly the two overloads in
  `PLAN.md` §1.2, matching the existing two-phase declaration style. Add whatever of
  `<type_traits>`, `<utility>`, `<initializer_list>` the build actually needs and record it.
- Then add an `or_construct.test.cpp` smoke target via `fvo_add_test` proving both overloads
  compile and run (e.g. `optional<int>` engaged/disengaged for the pack overload;
  `optional<std::vector<int>>` disengaged with `{1,2,3}` for the init-list overload).
- After Step 00 merges, the no-touch-the-header guardrail is back on for every later step.
