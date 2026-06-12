# Step 00 — Implement `or_construct` (both overloads) + smoke test

**Goal:** add the free `or_construct` function — a forwarded-arg-pack overload and a
`std::initializer_list` overload — to the header, and prove both compile and run with a
minimal smoke test. **This is the only step that edits the header.**

## The header change

File: `include/beman/free_value_or/value_or.hpp`, namespace `smd::free_value_or`.

Add exactly the two overloads from `PLAN.md` §1.2, matching the file's existing two-phase
style (forward-declare inside the namespace, define out-of-line with
`smd::free_value_or::` qualification — copy how `value_or` / `or_invoke` are written).
**Verified to compile/run on g++ 15.2.** The leading `class Ret = void` sentinel gives both
the payload-typed default (`or_construct(m, args...)`) and the explicit-result form
(`or_construct<R>(m, args...)`) from one overload; see `PLAN.md` §1.1–§1.2 for the rationale.

This exact two-phase arrangement is verified to compile/run on g++ 15.2 — forward decls
carry **no** defaults (matching the header's existing style, where `R`'s default lives on the
definition), and the out-of-line definitions supply both the leading `Ret = void` and the
trailing `R = …` defaults:

```cpp
// in-namespace forward declarations (alongside the existing three) — no defaults
template <class Ret, nullable T, class... Args, class R>
constexpr R or_construct(T&& m, Args&&... args);

template <class Ret, nullable T, class E, class... Args, class R>
constexpr R or_construct(T&& m, std::initializer_list<E> il, Args&&... args);

// out-of-line definitions — supply BOTH defaults here
template <class Ret = void, smd::free_value_or::nullable T, class... Args,
          class R = std::conditional_t<std::is_void_v<Ret>,
                                       std::remove_cvref_t<std::iter_reference_t<T>>, Ret>>
constexpr R smd::free_value_or::or_construct(T&& m, Args&&... args) {
    return bool(m) ? static_cast<R>(*m) : R(std::forward<Args>(args)...);
}

template <class Ret = void, smd::free_value_or::nullable T, class E, class... Args,
          class R = std::conditional_t<std::is_void_v<Ret>,
                                       std::remove_cvref_t<std::iter_reference_t<T>>, Ret>>
constexpr R
smd::free_value_or::or_construct(T&& m, std::initializer_list<E> il, Args&&... args) {
    return bool(m) ? static_cast<R>(*m) : R(il, std::forward<Args>(args)...);
}
```

- Add the includes the build actually needs: `<type_traits>` (`conditional_t`,
  `remove_cvref_t`, `is_void_v`), `<utility>` (`forward`), `<initializer_list>`. The header
  already has `<iterator>`. Build first; add only what's missing; record what was needed.
- Both defaults (`Ret = void` on the leading param, `R = conditional_t<…>` trailing) sit on
  the **out-of-line definition**, none on the forward decls — this matches the existing
  three functions' convention and is the arrangement that was verified to compile. `T` /
  `E` / `Args` are deduced, never defaulted.
- Do **not** add `reference_constructs_from_temporary_v` guards — the result is a prvalue
  value type and cannot dangle.
- Do **not** touch the three existing functions.

## Smoke test

`tests/beman/free_value_or/or_construct.test.cpp` (new exe via `fvo_add_test`, C++23).
Include `value_or.hpp` twice (include-guard idempotency check, matching the other test
files) + `test_types.hpp`. Minimal proof that **both overloads** compile and run:

```cpp
// pack overload, scalar, default Ret (R = payload type = int)
CHECK(fvo::or_construct(std::optional<int>{7}, 0)  == 7);   // engaged
CHECK(fvo::or_construct(std::optional<int>{},  5)  == 5);   // disengaged → int(5)
CHECK(fvo::or_construct(std::optional<int>{})      == 0);   // disengaged, zero args → int{}

// explicit Ret: payload int convertible to long
CHECK(fvo::or_construct<long>(std::optional<int>{7}, 0L) == 7L);  // engaged: int → long
CHECK(fvo::or_construct<long>(std::optional<int>{},  9L) == 9L);  // disengaged: long(9L)

// init-list overload (default Ret)
auto v = fvo::or_construct(std::optional<std::vector<int>>{}, {1, 2, 3});
CHECK(v == std::vector<int>{1, 2, 3});                       // disengaged → vector{1,2,3}
```

Add one `TEST_CASE` wrapping these `CHECK`s so ctest has a runnable entry.

## CMake

Add one `fvo_add_test(beman.free_value_or.tests.or_construct or_construct.test.cpp)` line to
`tests/beman/free_value_or/CMakeLists.txt`, in a clearly commented `or_construct Step 00`
block.

## Build & verify
C++23 configure/build/ctest (commands in `PLAN.md` §2). The new smoke test passes **and** all
46 existing tests still pass.

## Done criteria
- Both `or_construct` overloads compile; the init-list overload is selected for a
  braced-init-list / `initializer_list` argument and the pack overload otherwise (the smoke
  test exercises both); the explicit-`Ret` form (`or_construct<long>(...)`) compiles and
  routes correctly.
- Smoke test green; existing suite green; merged to `main`.

## Notes for your handoff
- The exact includes you had to add.
- Confirm overload resolution behaved (no ambiguity between the two overloads for the smoke
  cases, and the explicit-`Ret` arg binds to `Ret` not `T`) — Steps 01/02 lean on this.
- Whether the leading `Ret = void` default + the `R = conditional_t<…>` default compiled as
  written in the two-phase form for every smoke type, or needed adjustment (if it needed
  adjustment, that's an Issue — record it, don't silently re-spec).
