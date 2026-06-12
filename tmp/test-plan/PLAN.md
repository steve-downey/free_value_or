# Test Plan: `free_value_or` — `value_or`, `reference_or`, `or_invoke`, `nullable`

This is the **master plan**. It is read once at the start of every step for orientation.
Per-step detail lives in [`steps/`](steps/). Running state lives in
[`CHECKLIST.md`](CHECKLIST.md). Cross-agent knowledge lives in [`handoff/`](handoff/).

> **Audience:** a *fresh* Sonnet agent with no prior context. Read
> [`AGENT_PROMPT.md`](AGENT_PROMPT.md) first — it tells you exactly what to do.
> This file is the "why" and the big picture.

---

## 1. What is under test

File: `include/beman/free_value_or/value_or.hpp`, namespace `smd::free_value_or`.
Based on **WG21 P1255** (Steve Downey, *"A view of 0 or 1 elements"* / free `value_or`
family). P1255 motivates non-member, concept-constrained `value_or` that works uniformly
over *any* nullable (optional-like or pointer-like) type, and adds lazy (`or_invoke`) and
reference-preserving (`reference_or`) variants. **Test against the actual header
semantics below; consult the paper only for intent/naming.**

The interface (current source):

```cpp
template <class T>
concept nullable = requires(const T t) { bool(t); *(t); };

template <nullable T, class U, class R = std::common_reference_t<std::iter_reference_t<T>, U&&>>
constexpr auto reference_or(T&& m, U&& u) -> R;   // R is (usually) a reference

template <nullable T, class U, class R = std::common_type_t<std::iter_reference_t<T>, U&&>>
constexpr auto value_or(T&& m, U&& u) -> R;       // R is (usually) a value

template <nullable T, class I, class R = std::common_type_t<std::iter_reference_t<T>, std::invoke_result_t<I>>>
constexpr auto or_invoke(T&& m, I&& invocable) -> R;  // lazy: I() only if disengaged
```

Behavioral contract to confirm:

| Function | engaged (`bool(m)==true`) | disengaged | return kind | laziness |
|----------|---------------------------|------------|-------------|----------|
| `value_or`     | `*m` as value     | `u` as value          | `common_type`       | `u` eager |
| `reference_or` | `*m` as reference | `u` as reference      | `common_reference`  | `u` eager |
| `or_invoke`    | `*m`              | `invocable()`         | `common_type`       | `invocable()` **only when disengaged** |

`reference_or` carries two dangling guards:
`static_assert(!std::reference_constructs_from_temporary_v<R, U>)` and
`static_assert(!std::reference_constructs_from_temporary_v<R, T&>)`.

## 2. Standard-version reality (READ THIS — it shapes every step)

- `std::common_reference_t` / `std::common_type_t`: C++20.
- `std::reference_constructs_from_temporary_v`: **C++23**. The header's `reference_or`
  therefore *cannot compile* below C++23. → **All test targets build at C++23 minimum.**
- `std::expected`: C++23.
- `std::optional<T&>` (P2988): **C++26**, and only in a new-enough libstdc++.
  → `optional<T&>` test sections must be **feature-gated** (e.g.
  `#if defined(__cpp_lib_optional) && __cpp_lib_optional >= 202506L`) and exercised by a
  **separate C++26 test target**. The Step 00 agent must empirically determine what the
  local toolchain (GCC 15.2 / libstdc++) actually supports and record it in the handoff.

The default CMake presets compile at C++20 (`CMakePresets.json`) — do **not** rely on
them for the test build. Step 00 establishes test targets with explicit standards.

## 3. Test matrix

**Nullable types (must satisfy `nullable`, must work at runtime):**
`std::optional<T>`, `std::expected<T,E>`, raw pointer `T*`, `std::shared_ptr<T>`,
`std::unique_ptr<T>`, and (C++26-gated) `std::optional<T&>`.

**Non-nullable types (must NOT satisfy `nullable`):**
`int` (no `*`), `std::string` (no `bool`/`*` deref-to-value), `std::vector<int>`,
a bespoke "bool-only" type (has `operator bool`, no `operator*`), a bespoke
"deref-only" type (has `operator*`, no contextual bool).

**Value-category axes:** `m` as lvalue / rvalue / const; `u` as lvalue / rvalue;
engaged / disengaged.

**Dangling axis (negative-compile):** cases where `reference_or`'s `R` would bind to a
temporary (e.g. `optional<int>` with a `long`/`std::string`-from-literal default) must
fail the `reference_constructs_from_temporary_v` static_assert.

**Concept-violation axis (negative-compile):** calling any of the three with a
non-nullable first argument must fail to compile *because of the `nullable` constraint*.

## 4. How the steps are organized

Small, independently mergeable steps. Each step = one git worktree + one branch +
one merge to `main`. The whole sequence:

| Step | File | Outcome |
|------|------|---------|
| 00 | [steps/step-00-harness.md](steps/step-00-harness.md) | Test harness: CMake targets at C++23 (+ C++26 variant), shared `test_types.hpp` of model/anti-model types, negative-compile CMake helper, smoke build. |
| 01 | [steps/step-01-concept.md](steps/step-01-concept.md) | `nullable` concept: `static_assert` positive + negative coverage. |
| 02 | [steps/step-02-value_or.md](steps/step-02-value_or.md) | `value_or`: runtime + return-type + value-category coverage over all nullable types. |
| 03 | [steps/step-03-reference_or.md](steps/step-03-reference_or.md) | `reference_or`: reference identity, `common_reference`, value categories. |
| 04 | [steps/step-04-negative-compile.md](steps/step-04-negative-compile.md) | WILL_FAIL targets: dangling rejection + non-nullable-arg rejection, with diagnostic regexes. |
| 05 | [steps/step-05-or_invoke.md](steps/step-05-or_invoke.md) | `or_invoke`: results + **laziness** (invocable not called when engaged). |
| 06 | [steps/step-06-constexpr.md](steps/step-06-constexpr.md) | `constexpr` usage of all three in constant evaluation. |
| 07 | [steps/step-07-optional-ref-and-finalize.md](steps/step-07-optional-ref-and-finalize.md) | C++26 `optional<T&>` target + final consolidation, full-suite run, README/test notes. |

Steps are ordered by dependency (00 must be first; 07 last). Within that, each is sized
to be doable by one fresh agent in one sitting without exhausting context.

## 5. Per-step workflow (summary — full version in AGENT_PROMPT.md)

1. Read latest `handoff/handoff-*.md` (highest number) + this PLAN + your step file +
   `CHECKLIST.md`.
2. `git worktree add -b test/step-NN <path> main` and work there.
3. Implement the step. Build & run tests (commands in the step file).
4. Commit in the worktree, merge `--no-ff` into `main`, remove the worktree + branch.
5. Tick your step in `CHECKLIST.md`. Write `handoff/handoff-NNN-<slug>.md` for the next
   agent (what changed, what you learned, gotchas, what's next).
6. Stop. The next fresh agent takes over.

## 6. Guardrails

- **Do not modify** `include/beman/free_value_or/value_or.hpp` to make a test pass. If a
  test reveals a *bug* in the header, record it in the handoff and `CHECKLIST.md` "Issues"
  section — do **not** silently fix it. (A separate decision by the human owner.)
- Tests use **Catch2 v3** (`find_package(Catch2 3 REQUIRED)`, link
  `Catch2::Catch2WithMain`), matching the existing `tests/.../CMakeLists.txt`.
- Keep the existing `value_or.test.cpp` building until Step 02 replaces its placeholder.
- **Rename-proofing:** the namespace is `smd::free_value_or` today and will be renamed to
  `beman::free_value_or` later (the "great renaming", once correctness is established and
  before upstreaming / the "Free Value Or Else" paper). Tests must not spell `smd::` inline —
  refer to everything through the `fvo` alias defined in `test_types.hpp`
  (`namespace fvo = smd::free_value_or;`) so the rename is a one-line change. See Step 00.
- The `tmp/` plan dir is git-ignored and lives only in the **main** checkout. Always
  read/write it via its absolute path; worktrees won't contain it. Never commit it.
