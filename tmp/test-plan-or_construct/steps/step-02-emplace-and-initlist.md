# Step 02 — emplace-style multi-arg + `initializer_list` overload

**Goal:** exercise the surface that makes `or_construct` more than `value_or`: **in-place
construction** from zero, one, or many arguments (the pack overload), and from a
`std::initializer_list` with optional trailing args (the second overload). This is the
"convenience P3413 highlights and a lambda lacks" that motivates a dedicated function.

## File
`tests/beman/free_value_or/or_construct_construct.test.cpp` (new exe via `fvo_add_test`,
C++23). Include `value_or.hpp` twice + `test_types.hpp` + `<string>`, `<vector>`,
`<utility>`.

## Pack overload — construction arity
Use a disengaged nullable so the construction path runs; pick payload types whose
constructors make the arity observable.

1. **Zero args → default construction:** `or_construct(std::optional<std::string>{})` is the
   empty string; `or_construct(std::optional<int>{})` is `0`;
   `or_construct(std::optional<std::vector<int>>{})` is an empty vector. (`R{}` /
   `R(...)` with no args.)
2. **One arg:** `or_construct(std::optional<std::string>{}, "hi")` is `"hi"`.
3. **Multiple args → emplace-style:**
   - `or_construct(std::optional<std::string>{}, 3, 'x')` is `"xxx"` (the `string(size_t,
     char)` constructor).
   - `or_construct(std::optional<std::pair<int,int>>{}, 1, 2)` is `{1, 2}`.
4. **Engaged short-circuits construction of the result:** for each of the above, an *engaged*
   nullable returns the held value and the args do not appear in the result (e.g.
   `or_construct(std::optional<std::string>{"held"}, 3, 'x') == "held"`). (Full laziness —
   that the args' *side effects* don't happen — is Step 03; here just confirm the *result*.)
5. **Move-in argument:** construct from a moved-in lvalue, e.g.
   `std::string s = "move me"; auto r = or_construct(std::optional<std::string>{}, std::move(s));`
   `r == "move me"`. Confirms the pack forwards (`Args&&...` + `std::forward`).

## `initializer_list` overload
1. **Plain init-list:**
   - `or_construct(std::optional<std::vector<int>>{}, {1, 2, 3})` is `{1, 2, 3}`.
   - `or_construct(std::optional<std::string>{}, {'a', 'b', 'c'})` is `"abc"`
     (`string(initializer_list<char>)`).
2. **Init-list + trailing args:** a type with an `initializer_list`-plus-extra constructor,
   e.g. `std::vector<int>({1, 2, 3}, alloc)` —
   `or_construct(std::optional<std::vector<int>>{}, {1, 2, 3}, std::allocator<int>{})` is
   `{1, 2, 3}`. (If a clean allocator case is awkward on this toolchain, substitute any
   library type with an `initializer_list<E>, Arg` constructor and note the substitution.)
3. **Overload selection is correct:** a braced-init-list argument selects the init-list
   overload (the above all pass through it). Add a `static_assert` or a comment confirming the
   pack overload is *not* selected for `{...}` (a braced-init-list is non-deduced against
   `Args&&...`, so only the init-list overload is viable — note in handoff if the compiler
   says otherwise).
4. **Engaged short-circuit** for an init-list call too:
   `or_construct(std::optional<std::vector<int>>{{9}}, {1, 2, 3}) == std::vector<int>{9}`.

## Return type
`static_assert` that both overloads, with `Ret` **omitted**, return
`R = remove_cvref_t<iter_reference_t<T>>` (e.g. `std::vector<int>`, `std::string`,
`std::pair<int,int>`), independent of the argument shape. Add one explicit-`Ret` emplace
case to confirm it composes — e.g.
`or_construct<std::vector<long>>(std::optional<std::vector<long>>{}, {1L, 2L, 3L})` returns
`std::vector<long>` and equals `{1,2,3}`. (Behavioral depth of explicit `Ret` lives in
Step 01; here just confirm it works through the init-list / emplace overload too.)

## Build & verify
C++23 configure/build/ctest. All green.

## Done criteria
- Zero/one/many-arg pack construction, move-in forwarding, init-list and init-list+args all
  produce the expected values; engaged short-circuits to the held value in every case;
  return-type `static_assert`s hold.
- Merged to `main`.

## Notes for your handoff
- Any overload-resolution surprise between the two overloads (especially the braced-init-list
  case and the init-list + trailing-args case).
- The init-list-plus-trailing-args type you actually used (if you substituted for the
  allocator case).
