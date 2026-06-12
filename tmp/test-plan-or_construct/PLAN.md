# Test Plan: `free_value_or` — `or_construct` (the P3413 gap)

This is the **master plan** for adding and testing `or_construct`, the free-function
analogue of the one member function from Corentin Jabot's **P3413R0** (reviving Marc Mutz's
**P2218R0**) that the *Free Value Or Else* paper (D4270R0) currently has **no analogue**
for: P3413's `value_or_construct`. This plan is a *sibling* to `tmp/test-plan/` (the plan for
the three already-shipping functions `value_or` / `reference_or` / `or_invoke`) and is read
once at the start of every step for orientation.

Per-step detail lives in [`steps/`](steps/). Running state lives in
[`CHECKLIST.md`](CHECKLIST.md). Cross-agent knowledge lives in [`handoff/`](handoff/).

> **Audience:** a *fresh* agent with no prior context. Read
> [`AGENT_PROMPT.md`](AGENT_PROMPT.md) first — it tells you exactly what to do.
> This file is the "why" and the big picture.

> **Naming note:** the free function added here is named **`or_construct`** — the exact
> spelling D4270R0 itself proposes ("a free `or_construct` could be added later if wanted").
> P3413's *member* is named `value_or_construct`; references below to *P3413's member* keep
> that name, references to *our free function* use `or_construct`. Do not conflate the two.

---

## 0. The gap this plan closes

D4270R0's "Relation to P3413R0" chapter (`papers/free_value_or.tex`) lays out the gap
explicitly. P3413R0 proposes, **as members on `optional` and `expected`**:

- `value_or_construct(args...)` — construct the fallback `T` **lazily and in place** from
  forwarded arguments (never constructed on the engaged path), with `initializer_list`
  overloads;
- `value_or_else(callable)` — a lazily evaluated nullary callable.

D4270 already answers `value_or_else` with the free `or_invoke`. But for P3413's
`value_or_construct` the paper's own comparison table says:

> in-place fallback construction — P3413R0: `value_or_construct`; this paper: *via lambda;
> no direct analogue*

and the prose:

> P3413's `value_or_construct` has no direct analogue here; its in-place construction is
> expressible as `or_invoke(m, [&]{ return T(args...); })`, though **without the
> `initializer_list` convenience**, and a free `or_construct` could be added later if
> wanted.

**This plan builds that "later".** It adds a free `or_construct` — a **pair of overloads**,
one taking a forwarded argument pack, one taking a `std::initializer_list` (plus trailing
args) — to the reference implementation, and runs it through the **same test matrix**
already applied to `value_or` / `reference_or` / `or_invoke`.

**Downstream (not part of this plan):** once the implementation + tests land, D4270's
comparison-table row and the "no direct analogue" prose can be updated to point at the new
`or_construct`. That paper edit is a separate follow-up, flagged here so it is not forgotten.

---

## 1. What is under test — the API to add

File: `include/beman/free_value_or/value_or.hpp`, namespace `smd::free_value_or` (the
"great renaming" to `beman::free_value_or` is still pending — see Guardrails).

Unlike the three existing functions, **this function does not exist yet.** Step 00 adds it.
After Step 00 the no-touch-the-header guardrail applies again to every later step.

### 1.1 Proposed semantics

`or_construct<Ret>(m, args...)` returns a freshly built result `R` — from the contained
value of the nullable `m` when engaged, and otherwise **constructed in place** from
`args...`. It is the generalization of P3413's member `value_or_construct`, and it is
**lazy** like `or_invoke`: the fallback is constructed **only on the disengaged path**.

**The result type `R` is independent of the nullable's payload type.** The nullable holds
some `U` (where `iter_reference_t<T>` is `U&` / `const U&`); the function does **not** require
`U` to *be* `R`, only that `U` be **convertible to** `R`. `R` is chosen as follows:

- **`Ret` defaulted (the common case):** `or_construct(m, args...)` — `R` is the **decayed
  payload type** `std::remove_cvref_t<std::iter_reference_t<T>>`. This is the P3413-parity
  spelling: the result is the nullable's own value type, engaged returns a copy of `*m`,
  disengaged constructs that type from the args. No explicit type needed.
- **`Ret` given (the generalization):** `or_construct<R>(m, args...)` — the result is `R`;
  the engaged path is `static_cast<R>(*m)` (so `U` need only be **convertible to** `R`), and
  the disengaged path is `R(args...)`. This lets the held payload type and the result type
  differ — e.g. `or_construct<std::string>(opt_string_view, "fallback")`.

Mechanically (see §1.2) `R` is a trailing computed parameter, defaulted from `Ret`:
`R = is_void_v<Ret> ? remove_cvref_t<iter_reference_t<T>> : Ret`.

This differs from `value_or` / `or_invoke`, which compute `R` as a **`common_type`** of the
payload and a *second, independent* type (`U` or `invoke_result_t<I>`). `or_construct` has no
such single second type to take a common type with — there is a whole **argument pack** that
*constructs* the result. So instead of deducing a common type, the result type is either the
payload type (default) or **named explicitly** by the caller, and the args construct it. This
is the key design point; record any surprise here in the handoff.

**Convertibility is the requirement on the nullable.** `static_cast<R>(*m)` enforces that
`U` is convertible to `R`; an `or_construct<R>` call whose payload is *not* convertible to
`R` is ill-formed (verified: trips `invalid static_cast` on g++ 15.2).

**Decision (R0): this is a Mandates-style hard error, not a Constraint — and not even an
explicit `static_assert`; the natural in-body `static_cast<R>(*m)` is the diagnostic.**
Rationale, recorded so the choice stays visible rather than implicit:

- *SFINAE buys nothing here.* A constraint is worth it only when removing the candidate lets
  overload resolution pick a *better* alternative. `or_construct`'s two overloads (pack vs
  `initializer_list`) are not competing fallbacks for the same call, so a
  `requires constructible_from<R, iter_reference_t<T>>` constraint would redirect nothing —
  it would only turn a precise "`std::string` is not convertible to `int`" into a vague "no
  matching function for call to `or_construct`". Strictly worse for the one caller who hit it.
- *Consistency with the family.* `value_or` / `or_invoke` already let a non-convertible
  alternative fail in-body at `static_cast<R>(…)`, with no constraint and no guard;
  `or_construct` does the same. `reference_or` is the lone exception that carries explicit
  `static_assert`s — and only because its failure mode (dangling) is one the language would
  *silently accept*. Convertibility is not like that: the `static_cast` already rejects it
  loudly, so an added `static_assert` would be redundant *and* out of step with `value_or`.

This mirrors the paper's stated position for `reference_or` ("a dangling call is a bug to be
reported, not a candidate to be quietly discarded") applied to the convertibility case. LEWG
may of course revisit; if it ever moves to a Constraint, Step 04's negative-compile regex
changes from the `static_cast` family to the constraint-failure family.

Both branches yield an `R` prvalue, so the result **never dangles** — like `value_or` and
`or_invoke`, and unlike `reference_or`, there is no reference in the result to check. (No
`reference_constructs_from_temporary_v` guards are needed or wanted.)

### 1.2 Proposed implementation (Step 00 adds exactly this)

Verified to compile and run on g++ 15.2 (`-std=c++23`). The leading `class Ret = void`
sentinel is the mechanism that gives both ergonomics in one overload: omit it and `R` is the
payload type; supply `or_construct<R>` and `R` is what you asked for. (A defaulted template
parameter preceding the deduced `T`/`Args...` is legal for *function* templates — the
trailing parameters are deduced from the call.)

```cpp
template <class Ret = void,
          nullable T,
          class... Args,
          class R = std::conditional_t<std::is_void_v<Ret>,
                                       std::remove_cvref_t<std::iter_reference_t<T>>,
                                       Ret>>
constexpr R or_construct(T&& m, Args&&... args) {
    return bool(m) ? static_cast<R>(*m)
                   : R(std::forward<Args>(args)...);
}

template <class Ret = void,
          nullable T,
          class E,
          class... Args,
          class R = std::conditional_t<std::is_void_v<Ret>,
                                       std::remove_cvref_t<std::iter_reference_t<T>>,
                                       Ret>>
constexpr R or_construct(T&& m, std::initializer_list<E> il, Args&&... args) {
    return bool(m) ? static_cast<R>(*m)
                   : R(il, std::forward<Args>(args)...);
}
```

Notes for the implementer:

- **Headers to add:** `value_or.hpp` already includes `<iterator>`. Both overloads need
  `std::conditional_t` / `std::remove_cvref_t` / `std::is_void_v` (`<type_traits>`) and
  `std::forward` (`<utility>`); the `initializer_list` overload needs `<initializer_list>`.
  Build first; add only what's missing; record what was needed in the handoff.
- **Two-phase declaration style:** match the file's existing pattern (forward-declare inside
  the namespace, then define out-of-line with `smd::free_value_or::` qualification). See how
  `value_or` / `or_invoke` are written and follow suit exactly, for both overloads. Note this
  function uses a **leading explicit `Ret`** parameter, unlike the existing three (whose only
  explicit-capable parameter is `T`); keep that in mind when transcribing the two-phase form.
- **Return type spelled `-> R` vs trailing `R`:** the existing functions use
  `-> R` trailing-return. Either spelling is fine here; match the file for consistency and
  record which you used.
- **Overload resolution:** the `initializer_list` overload is *more specialized*, so a call
  passing a real `std::initializer_list` object, or a braced-init-list
  `or_construct(m, {1,2,3})`, selects it; a call with ordinary arguments selects the pack
  overload. A zero-arg call `or_construct(m)` selects the pack overload and
  **default-constructs** `R` on the disengaged path — a legitimate, useful case. With an
  explicit `Ret`, `or_construct<R>(m, {1,2,3})` still routes to the init-list overload (the
  explicit arg binds to `Ret`, `T`/`E`/`Args` are deduced) — verified.
- **Engaged path** is `static_cast<R>(*m)`: it *observes* (builds a fresh `R` from `*m`), it
  does **not** consume — identical to `value_or`'s "observe, don't consume" rule. An rvalue
  nullable does not move its payload out. When `R` differs from the payload type, this is a
  genuine **conversion** of `*m` into `R`; when `R` is the payload type it is a copy.
  Document this in tests, do not try to "fix" it.
- **Guardrail flips after Step 00:** this is the *only* step that edits the header. If a
  later step's test reveals the *design above* is wrong (not just a typo), STOP and record
  it in `CHECKLIST.md` "Issues" + your handoff — do not silently re-spec the function.

---

## 2. Standard-version reality (READ THIS — it shapes every step)

Identical to the sibling plan; restated so this plan is self-contained:

- `std::common_type_t` / `std::common_reference_t`: C++20 (not needed by this function, but
  the header as a whole uses them).
- `std::remove_cvref_t`: C++20.
- `std::reference_constructs_from_temporary_v`: **C++23** — used by the header's
  `reference_or`, so the header *cannot compile below C++23*. → **All test targets build at
  C++23 minimum**, exactly as the existing suite does.
- `std::expected`: C++23.
- `std::optional<T&>` (P2988): **C++26** and only in a new-enough libstdc++. The local
  libstdc++ (GCC 15.2, `__cpp_lib_optional = 202110`) does **not** provide it; the suite
  vendors `beman::optional26` and exposes `fvo_opt::optional<T&>` with
  `FVO_HAS_OPTIONAL_REF=1` injected for every positive test (see sibling plan's
  `CHECKLIST.md` toolchain facts). The `optional<T&>` section is handled the same way here
  (Step 06).

Build everything with the existing helpers and command (verbatim from the sibling suite):

```bash
cmake -B build -S . -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=./infra/cmake/use-fetch-content.cmake
cmake --build build
ctest --test-dir build --output-on-failure
```

The `fvo_add_test(name source)` and `fvo_add_compile_fail_test(test_name source regex)`
helpers in `tests/beman/free_value_or/CMakeLists.txt` already exist — **reuse them**, do not
reinvent.

---

## 3. Test matrix — the same matrix the main APIs run

The existing suite (sibling plan, steps 01–07) covers, per function: the `nullable` concept,
runtime engaged/disengaged over every nullable type, return type, value categories,
negative-compile (concept + dangling), laziness (`or_invoke`), constant evaluation, and the
C++26 `optional<T&>` target. `or_construct` runs the **same matrix**, minus the parts that do
not apply, plus the parts unique to in-place construction:

**Nullable types (must work at runtime):** `std::optional<T>`, `std::expected<T,E>`, raw
pointer `T*`, `std::shared_ptr<T>`, `std::unique_ptr<T>`, and (C++26-gated)
`std::optional<T&>` — the identical set the main APIs use. (`unique_ptr` engaged is the
rvalue case, as in `value_or.test.cpp`.)

**Value-category axes:** `m` as lvalue / const lvalue / rvalue; engaged / disengaged. (The
fallback "value-category of `u`" axis becomes "value categories of the *constructor
arguments*" — lvalue args, rvalue/temporary args, and a moved-in argument.)

**Payload axes:** `int` (default/scalar construction), `std::string` (single-arg, multi-arg
`(count, char)` emplace-style, and `initializer_list<char>`), and at least one
`initializer_list`-constructible container, e.g. `std::vector<int>` from `{1,2,3}`.

**Construction-surface axes (unique to this function):**
- **zero args** → default-constructed fallback (`or_construct(m)`);
- **one arg** → single-value construction (the plain P3413 case);
- **multiple args** → emplace-style in-place construction (e.g. `std::string(3, 'x')`,
  `std::pair<int,int>(1, 2)`);
- **`initializer_list`** → the convenience P3413 highlights and `or_invoke` "lacks"
  (`std::vector<int>{1,2,3}`, `std::string{'a','b','c'}`);
- **`initializer_list` + trailing args** → e.g. `std::vector<int>({1,2,3}, alloc)`.

**Result-type axes (`Ret` parameter):**
- **`Ret` defaulted** → `R` is the decayed payload type (P3413 parity); engaged returns a
  copy of `*m`.
- **`Ret` explicit, payload convertible to `R`** → e.g. `or_construct<long>(opt_int, 0L)`
  (engaged converts `int`→`long`), `or_construct<std::string>(opt_string_view, "x")` (engaged
  converts the held `string_view`→`std::string`, disengaged builds the string from args).
  This is the axis that exercises "payload `U` need only be *convertible to* `R`."
- **`Ret` explicit, payload NOT convertible to `R`** → negative-compile (e.g.
  `or_construct<int>(opt_string, 0)`): ill-formed at the engaged `static_cast<R>(*m)`.

**Laziness axis (the headline):** the fallback is constructed **only when disengaged**. This
is `or_construct`'s entire reason to exist over `value_or`, so it is tested as emphatically
as `or_invoke`'s laziness: a payload type with an instrumented constructor must register
**zero** constructions on the engaged path and **exactly one** on the disengaged path, for
**both** overloads.

**Return-type axis:** `static_assert(std::is_same_v<decltype(or_construct(m, args...)),
std::remove_cvref_t<std::iter_reference_t<T>>>)` for each nullable type and each overload.

**Concept-violation axis (negative-compile):** calling either overload with a non-nullable
first argument must fail *because of the `nullable` constraint* (same diagnostic family the
existing `*_non_nullable_fail.cpp` TUs match).

**Constant-evaluation axis:** `or_construct` is `constexpr`; `static_assert` it in constant
context for engaged + disengaged, with a `constexpr`-constructible payload.

**Not applicable:** there is no `reference_or`-style variant and no dangling axis — the
result is always a prvalue of a decayed value type, so it cannot dangle. State this
explicitly in the finalize README rather than leaving the reader to wonder.

---

## 4. How the steps are organized

Small, independently mergeable steps. Each step = one git worktree + one branch + one merge
to `main`. The whole sequence:

| Step | File | Outcome |
|------|------|---------|
| 00 | [steps/step-00-impl.md](steps/step-00-impl.md) | **Implement** both `or_construct` overloads in the header; add an `or_construct.test.cpp` target + a smoke check that both overloads compile and run. The only step that edits the header. |
| 01 | [steps/step-01-behavior.md](steps/step-01-behavior.md) | Runtime engaged/disengaged + return-type + value categories over all nullable types (single-arg pack overload). Mirrors sibling Step 02. |
| 02 | [steps/step-02-emplace-and-initlist.md](steps/step-02-emplace-and-initlist.md) | The distinctive surface: zero-arg/default, multi-arg emplace construction, and the `initializer_list` overload (and init-list + trailing args). |
| 03 | [steps/step-03-laziness.md](steps/step-03-laziness.md) | **Laziness** — fallback constructed *only* when disengaged, for both overloads. The headline. Mirrors sibling Step 05. |
| 04 | [steps/step-04-negative-compile.md](steps/step-04-negative-compile.md) | WILL_FAIL targets: non-nullable first argument rejected by `nullable`, for both overloads; optional "no viable constructor from args" documentation case. Mirrors sibling Step 04. |
| 05 | [steps/step-05-constexpr.md](steps/step-05-constexpr.md) | `constexpr` constant-evaluation for both overloads. Mirrors sibling Step 06. |
| 06 | [steps/step-06-optional-ref-and-finalize.md](steps/step-06-optional-ref-and-finalize.md) | C++26 `optional<T&>` coverage + final consolidation, full-suite run, test README update. Mirrors sibling Step 07. |

Steps are ordered by dependency (00 must be first; 06 last). Within that, each is sized to
be doable by one fresh agent in one sitting without exhausting context.

---

## 5. Per-step workflow (summary — full version in AGENT_PROMPT.md)

1. Read latest `handoff/handoff-*.md` (highest number) + this PLAN + your step file +
   `CHECKLIST.md`. Also skim the **sibling** plan's latest handoff
   (`../test-plan/handoff/`) for toolchain facts (compiler version, build command,
   diagnostic regexes) — they apply unchanged.
2. `git worktree add -b or_construct/step-NN <path> main` and work there.
3. Implement the step. Build & run tests (commands in §2 / the step file).
4. Commit in the worktree, merge `--no-ff` into `main`, remove the worktree + branch.
5. Tick your step in `CHECKLIST.md`. Write `handoff/handoff-NNN-<slug>.md` for the next
   agent (what changed, what you learned, gotchas, what's next).
6. Stop. The next fresh agent takes over.

---

## 6. Guardrails

- **Step 00 is the only step that edits `include/beman/free_value_or/value_or.hpp`**, and it
  edits it to add exactly the two overloads specified in §1.2 — nothing else. After Step 00,
  the sibling plan's guardrail is back in force: **do not modify the header to make a test
  pass.** If a test reveals a *bug* or a *design flaw* in `or_construct`, record it in the
  handoff and `CHECKLIST.md` "Issues" — do **not** silently change the semantics. (A
  separate decision by the human owner / a paper question.)
- **Do not touch the three existing functions** or their tests. This plan is additive. Keep
  the existing 46-test suite green at every merge.
- Tests use **Catch2 v3** (`find_package(Catch2 3 REQUIRED)`, link
  `Catch2::Catch2WithMain`), via the existing `fvo_add_test` / `fvo_add_compile_fail_test`
  helpers. One executable per `.test.cpp` file. Every new file starts with
  `// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception`.
- **Rename-proofing:** the namespace is `smd::free_value_or` today and will be renamed to
  `beman::free_value_or` later. Tests must not spell `smd::` inline — refer to everything
  through the `fvo` alias from `test_types.hpp` (`namespace fvo = smd::free_value_or;`), i.e.
  write `fvo::or_construct`. For a negative-compile TU that can't include `test_types.hpp`,
  put the same one-line alias at the top of the TU.
- The `tmp/` plan dirs are git-ignored and live only in the **main** checkout. Always
  read/write this plan via its absolute path; worktrees won't contain it. Never commit it.
- **Stale path warning:** the old sibling `AGENT_PROMPT.md` references
  `/home/sdowney/src/free_value_or/free_value_or` — that path is now the *bare* repo
  (`free_value_or.git`). The live main checkout is
  **`/home/sdowney/src/free_value_or/main`**. Use that. A leftover `fvo-step-04` worktree
  may exist; ignore it (don't reuse its branch name).
