# Step 03 — laziness: the fallback is constructed only when disengaged

**Goal — the headline.** `or_construct`'s entire reason to exist over `value_or` is that the
fallback object is **not constructed at all** on the engaged path (P3413's whole point). This
step proves it as emphatically as sibling Step 05 proves `or_invoke`'s laziness, for **both**
overloads.

> Contrast to keep in mind: `value_or(m, u)` evaluates `u` eagerly (it's an ordinary
> argument) — sibling Step 02 confirmed a fallback lambda is invoked even when engaged.
> `or_construct(m, args...)` must *not* construct the result object when engaged. The `args`
> themselves are still ordinary arguments (they are evaluated at the call site); what is lazy
> is the **construction of `R` from them**. Test the construction, and say this clearly.

## File
`tests/beman/free_value_or/or_construct_laziness.test.cpp` (new exe via `fvo_add_test`,
C++23). Include `value_or.hpp` twice + `test_types.hpp`.

## Instrumented payload
Define a payload type with an observable constructor counter (a `static int` or a counter
referenced through a global), counting **constructions of the payload type itself** — the
object `R` that `or_construct` builds on the disengaged path. For example:

```cpp
struct Tracked {
    static inline int ctor_count = 0;
    int v = 0;
    Tracked() { ++ctor_count; }
    explicit Tracked(int x) : v(x) { ++ctor_count; }
    Tracked(std::initializer_list<int> il) : v((int)il.size()) { ++ctor_count; }
    // copy/move as needed; if you count copies/moves too, account for the engaged
    // static_cast<R>(*m) copy in your expectations and document it.
};
```

Reset `ctor_count = 0` before each call. Be precise about what you count: the **disengaged**
path constructs the fallback `R` (count goes up); the **engaged** path performs
`static_cast<R>(*m)`, which is a *copy/conversion* of the held value — if your counter also
counts copies, the engaged path will show one construction *from the copy*, not from the
arguments. Prefer counting only the arg-taking / default / init-list constructors (not the
copy/move ctor) so "fallback constructed" is unambiguous; or count separately. State your
choice in a comment and the handoff.

## Cases (both overloads)

### Pack overload
1. **Engaged → fallback NOT constructed:** an engaged nullable holding a `Tracked`; call
   `or_construct(engaged, 7)`; assert the **fallback** constructor (the `Tracked(int)` /
   default / init-list ctor) ran **zero** times. The result equals the held value.
2. **Disengaged → fallback constructed exactly once:** `or_construct(disengaged, 7)`; assert
   the fallback constructor ran **exactly once**; result `.v == 7`.
3. Repeat (1)/(2) for **zero-arg** (`or_construct(m)` → default ctor) and **multi-arg** forms
   if your `Tracked` supports them.

### `initializer_list` overload
4. **Engaged → init-list fallback NOT constructed:** engaged `optional<Tracked>`;
   `or_construct(engaged, {1, 2, 3})`; the `initializer_list` constructor ran **zero** times.
5. **Disengaged → constructed once:** `or_construct(disengaged, {1, 2, 3})`; the init-list
   ctor ran **exactly once**.

### Across nullable types
6. Repeat the engaged-zero / disengaged-one assertion for at least one more nullable type
   beyond `optional` (e.g. `expected<Tracked,int>` and a raw `Tracked*`), to show laziness is
   a property of the function, not of `optional`. (`unique_ptr<Tracked>` engaged via rvalue is
   a good move-only check; `shared_ptr` too.)

## Build & verify
C++23 configure/build/ctest. All green. These counter assertions are the ones most likely to
catch a real regression — make them unmistakable (clear messages, reset between cases).

## Done criteria
- For both overloads and across multiple nullable types: fallback construction count is **0**
  when engaged and **1** when disengaged. Results are correct.
- Merged to `main`.

## Notes for your handoff
- Exactly what your counter counts (arg ctors only vs. incl. copy/move), and how you handled
  the engaged-path `static_cast<R>(*m)` copy so it doesn't pollute the "fallback constructed"
  count.
- Confirm/deny that the engaged path truly skips the fallback ctor on this toolchain (it
  should — it's a ternary, the untaken branch is not evaluated). If the compiler did anything
  surprising under optimization, note it.
