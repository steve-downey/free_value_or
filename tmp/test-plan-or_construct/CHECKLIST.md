# Checklist — running state (`or_construct`)

The **first unticked box is the current step.** Tick a box only after the step's branch is
merged into `main` and the suite builds from the main checkout (existing 46 tests + the new
`or_construct` tests all green).

- [x] **Step 00** — Implement both `or_construct` overloads in the header + smoke test —
      `steps/step-00-impl.md` **(only step that edits the header)**
- [x] **Step 01** — `or_construct` runtime + return-type + value categories (single-arg) —
      `steps/step-01-behavior.md`
- [x] **Step 02** — zero-arg/default + multi-arg emplace + `initializer_list` overload —
      `steps/step-02-emplace-and-initlist.md`
- [x] **Step 03** — laziness (construct only when disengaged), both overloads —
      `steps/step-03-laziness.md`
- [x] **Step 04** — negative-compile (non-nullable arg, both overloads) —
      `steps/step-04-negative-compile.md`
- [x] **Step 05** — `constexpr` constant-evaluation, both overloads —
      `steps/step-05-constexpr.md`
- [x] **Step 06** — `optional<T&>` (C++26) + finalize/consolidate + README —
      `steps/step-06-optional-ref-and-finalize.md`

## Toolchain facts (inherited from the sibling plan; amend if anything differs)

- Compiler / libstdc++: `g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0`; `clang++` NOT found.
- Build command (use from any worktree):
  ```bash
  cmake -B build -S . -DCMAKE_CXX_STANDARD=23 \
    -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=./infra/cmake/use-fetch-content.cmake
  cmake --build build
  ctest --test-dir build --output-on-failure
  ```
- `std::expected`, `std::optional`, `std::expected` ops all `constexpr` at C++23: YES.
- `std::optional<T&>` NOT in libstdc++ (`__cpp_lib_optional = 202110`); vendored
  `beman::optional26` provides it, `FVO_HAS_OPTIONAL_REF=1` injected by `fvo_add_test`.
  Use `fvo_opt::optional<T&>` (alias in `test_types.hpp`).
- `std::unique_ptr` / `std::shared_ptr` are **not** usable in constant expressions — for the
  constexpr step stick to `optional` / `expected` / raw pointers.
- Negative-compile diagnostic for a failed `nullable` constraint on this toolchain:
  `"no matching function for call to"` (regex used by the sibling `*_non_nullable_fail`
  tests). Confirm it still applies to `or_construct` (two overloads → message may mention
  both candidates; widen the regex only if needed).
- As of the sibling suite's Step 06 merge, **46/46** tests pass. Baseline to preserve.

## Issues / suspected design flaws or header bugs (do NOT silently fix; record here)

_(none yet)_

## Notes carried across steps

- `or_construct<Ret>` result type: `Ret` defaulted (`void` sentinel) → `R` is the decayed
  payload type `remove_cvref_t<iter_reference_t<T>>` (P3413 parity); `Ret` explicit → `R` is
  `Ret`, and the payload need only be **convertible** to `R` (engaged is `static_cast<R>(*m)`).
  No `common_type` (there is no single independent second type — there's an arg pack). No
  dangling axis. Signature verified on g++ 15.2 (see handoff-000).
- `fvo_add_test(name source)` — 3-line positive test add.
- `fvo_add_compile_fail_test(test_name source regex)` — adds OBJECT lib + ctest entry +
  `PASS_REGULAR_EXPRESSION` check.
- `test_types.hpp` defines `namespace fvo = smd::free_value_or;` and (when
  `FVO_HAS_OPTIONAL_REF=1`) `namespace fvo_opt = beman::optional;`. Use `fvo::or_construct`.
- The `nullable` concept checks `const T` — types with non-const `operator bool`/`operator*`
  do not satisfy it (`nonconst_nullable` in `test_types.hpp`).
- Leftover unrelated worktree `fvo-step-04` may exist — ignore it.
