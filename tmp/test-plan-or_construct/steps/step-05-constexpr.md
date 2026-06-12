# Step 05 — `constexpr` / constant-evaluation coverage (both overloads)

**Goal:** `or_construct` is declared `constexpr`; confirm both overloads actually work in a
constant-evaluated context (not merely that they compile at runtime). Mirrors sibling
Step 06.

## File
`tests/beman/free_value_or/or_construct_constexpr.test.cpp` (new exe via `fvo_add_test`,
C++23). Include `value_or.hpp` twice + `test_types.hpp`.

## Approach
The strongest form is `static_assert(... == expected);` at file scope — that *forces*
constant evaluation. Use `constexpr` variables / helpers.

## Cases

### Pack overload
1. `static_assert(fvo::or_construct(std::optional<int>{7}, 0) == 7);` (engaged — fallback not
   constructed).
2. `static_assert(fvo::or_construct(std::optional<int>{}, 5) == 5);` (disengaged → `int(5)`).
3. `static_assert(fvo::or_construct(std::optional<int>{}) == 0);` (disengaged, **zero args** →
   default `int{}`).
4. `static_assert(fvo::or_construct(std::expected<int,int>{std::unexpected(0)}, 9) == 9);`
   (disengaged via `expected`).
5. A **raw-pointer** case in constant context: pointer to a `constexpr` static/`constinit`
   `int`, engaged; plus the `nullptr` disengaged case `or_construct((int*)nullptr, 4) == 4`.
   (Raw pointers are usable in constant expressions when pointing at a permitted constexpr
   object — sibling Step 06 confirmed this works on the toolchain. If pointer-in-constexpr is
   fiddly, use `optional`/`expected` and note it.)
6. **Explicit `Ret` in constant context:**
   `static_assert(fvo::or_construct<long>(std::optional<int>{7}, 0L) == 7L);` (engaged: int→long)
   and `static_assert(fvo::or_construct<long>(std::optional<int>{}, 9L) == 9L);` (disengaged).

### `initializer_list` overload
6. A `constexpr`-friendly init-list construction. `std::vector`/`std::string` are **not**
   usable in `static_assert` constant context here (heap), so use a payload whose
   `initializer_list` constructor is `constexpr` and non-allocating — e.g. `std::array`-like
   aggregate is awkward; the simplest is a tiny **local literal type**:

   ```cpp
   struct Sum {
       int total = 0;
       constexpr Sum() = default;
       constexpr Sum(std::initializer_list<int> il) {
           for (int x : il) total += x;
       }
       constexpr bool operator==(const Sum&) const = default;
   };
   static_assert(fvo::or_construct(std::optional<Sum>{}, {1, 2, 3}) == Sum{ {1,2,3} });
   static_assert(fvo::or_construct(std::optional<Sum>{Sum{}}, {1, 2, 3}) == Sum{});  // engaged
   ```

   (Adjust to whatever compiles cleanly; the point is a `constexpr` init-list construction on
   the disengaged path and a short-circuit on the engaged path.)

Add a trivial `TEST_CASE` (e.g. `SUCCEED()`) so ctest has a runnable entry; the real proof is
the `static_assert`s compiling.

## Notes / gotchas
- `std::optional` / `std::expected` ops are `constexpr` in C++23 — fine.
- `std::unique_ptr` / `std::shared_ptr` / heap-allocating `std::vector` / `std::string` are
  **not** usable in constant expressions — don't try. Stick to `optional`/`expected`, raw
  pointers, and small literal payload types.

## Build & verify
C++23 configure/build/ctest. The TU must compile (static_asserts hold); existing suite green.

## Done criteria
- `constexpr` `static_assert`s for **both overloads**, engaged + disengaged (incl. zero-arg
  and init-list), compile. Merged to `main`.

## Notes for your handoff
- Which payload types you could and couldn't use in constant context (esp. for the init-list
  overload), and the literal type you settled on.
