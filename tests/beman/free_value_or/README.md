# `free_value_or` test suite

Tests for `beman::free_value_or` — the non-member `value_or`, `reference_or`, and
`or_invoke` family (WG21 P1255 / D4270R0).  All tests use **Catch2 v3** and build at
**C++23 minimum** (required by `reference_or`'s `reference_constructs_from_temporary_v`
guards).

## Files

| File | What it covers |
|------|----------------|
| `smoke.test.cpp` | Minimal compile-and-run check at C++23; proves `beman::optional<int&>` links |
| `concept.test.cpp` | `nullable` concept: `static_assert` positive coverage (all model types) and negative coverage (non-nullable anti-models) |
| `value_or.test.cpp` | `value_or` runtime behaviour, return-type (`common_type`), value-category axes, all nullable types |
| `reference_or.test.cpp` | `reference_or` reference identity, `common_reference` return type, const propagation, mutation round-trips, all nullable types |
| `or_invoke.test.cpp` | `or_invoke` results, laziness (invocable not called when engaged), move-only and stateful invocables |
| `constexpr.test.cpp` | `static_assert`-level constant-evaluation of all three functions with `optional`, `expected`, and raw pointers |
| `optional_ref.test.cpp` | All three functions with `fvo_opt::optional<int&>` (C++26 reference-optional via vendored `beman::optional`): `nullable` static_assert, return-type proofs, reference identity, mutation, laziness, rebinding semantics |
| `or_construct.test.cpp` | `or_construct` Step 00 smoke: both overloads compile and run |
| `or_construct_behavior.test.cpp` | `or_construct` return type, engaged/disengaged, value categories, inward conversion, explicit `Ret` |
| `or_construct_construct.test.cpp` | `or_construct` zero-arg default-construction, multi-arg emplace-style, init-list overload |
| `or_construct_laziness.test.cpp` | `or_construct` laziness: fallback constructed exactly once when disengaged, not constructed when engaged |
| `or_construct_constexpr.test.cpp` | `or_construct` `static_assert`-level constant evaluation: both overloads, `optional`, `expected`, raw pointers, explicit `Ret` |
| `or_construct_optional_ref.test.cpp` | `or_construct` with `optional<int&>` and `optional<string&>`: nullable proof, `R = int` return-type proof, engaged/disengaged, explicit `Ret`, init-list, rebinding |
| `fail_not_nullable.cpp` | Negative compile: calling `value_or` with a non-nullable first arg must fail (`no matching function`) |
| `ref_or_temp_from_prvalue_fail.cpp` | Negative compile: `reference_or` with prvalue fallback that would dangle must fail (`static assertion failed`) |
| `ref_or_rvalue_string_fail.cpp` | Negative compile: `reference_or` with a `string`-from-literal fallback that would dangle must fail |
| `value_or_non_nullable_fail.cpp` | Negative compile: `value_or` with non-nullable first arg |
| `reference_or_non_nullable_fail.cpp` | Negative compile: `reference_or` with non-nullable first arg |
| `or_invoke_non_nullable_fail.cpp` | Negative compile: `or_invoke` with non-nullable first arg |
| `or_construct_non_nullable_fail.cpp` | Negative compile: `or_construct` (pack) with non-nullable first arg |
| `or_construct_initlist_non_nullable_fail.cpp` | Negative compile: `or_construct` (init-list) with non-nullable first arg |
| `or_construct_payload_not_convertible_fail.cpp` | Negative compile: `or_construct` explicit `Ret` with payload not convertible to `Ret` |

Shared infrastructure:

- `test_types.hpp` — namespace alias `fvo = smd::free_value_or` (rename-proofing; update
  this one line when the library moves to `beman::`), `fvo_opt = beman::optional` alias,
  `NullableFixture<T>` helpers, and anti-model types (`bool_only`, `deref_only`,
  `nonconst_nullable`).
- `CMakeLists.txt` — `fvo_add_test` and `fvo_add_compile_fail_test` helpers; each test
  gets `FVO_HAS_OPTIONAL_REF=1` injected and links `beman::optional`.

## Standard-version notes

- **C++23 baseline** — all targets build at `-std=c++23`.  `reference_or` cannot compile
  below C++23 (`reference_constructs_from_temporary_v` requires C++23).
- **`std::optional<T&>` (P2988 / C++26)** — NOT available on this toolchain
  (`libstdc++ 15.2`, `__cpp_lib_optional = 202110`).  The `optional<T&>` tests in
  `optional_ref.test.cpp` use the vendored **`beman::optional`** library instead, which is
  always available via `fvo_opt::optional<T&>`.  Tests are guarded by `#if FVO_HAS_OPTIONAL_REF`.

## How to run

```bash
cmake -B build -S . -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=./infra/cmake/use-fetch-content.cmake
cmake --build build
ctest --test-dir build --output-on-failure
```

The negative-compile tests run as part of `ctest` automatically (they are WILL_FAIL tests
that pass by failing to compile with the expected diagnostic).

## `or_construct`

`or_construct` is the free-function analogue of P3413R0's member `value_or_construct` — the
gap that D4270R0 explicitly leaves open.  It provides **lazy, in-place fallback construction**:
when the nullable is disengaged the fallback is constructed from the supplied arguments; when
engaged the held value is returned as a copy.

### Two overloads

```cpp
// Pack overload: construct R from a variadic argument pack (including zero args)
template <class Ret = void, nullable T, class... Args>
constexpr R or_construct(T&& m, Args&&... args);

// initializer_list overload: construct R from an init-list + optional trailing args
template <class Ret = void, nullable T, class E, class... Args>
constexpr R or_construct(T&& m, std::initializer_list<E> il, Args&&... args);
```

### Result type

| `Ret` argument | `R` (the return type) |
|----------------|----------------------|
| Omitted (default `void`) | `remove_cvref_t<iter_reference_t<T>>` — the **decayed** payload type |
| Explicit type | `Ret` — the payload need only be **convertible** to `Ret` via `static_cast` |

Unlike `value_or` there is **no `common_type` negotiation** — there is no single independent
second type; the fallback is an arg pack that is used to construct `R` directly.

For `optional<int&>`: `iter_reference_t` = `int&`, so default `R = remove_cvref_t<int&> = int`
(always a value, never a reference).

### Key properties

- **Laziness (the headline):** the fallback is constructed only when the nullable is disengaged.
  Arguments are perfect-forwarded into `R(std::forward<Args>(args)...)` — no copy is made when
  the optional is engaged.

- **Inward construction vs outward promotion:** `value_or` uses `common_type` to find a shared
  type that both branches can convert *to*.  `or_construct` inverts this: you specify the target
  type `R`, and both the held value (`static_cast<R>(*m)`) and the fallback (`R(args...)`) must
  be convertible *to* `R`.  There is no promotion negotiation.

- **Cannot dangle:** the return is always a prvalue of the decayed value type.  There is no
  dangling-reference family of negative-compile tests (contrast `reference_or`, which can
  produce a reference and carries two `reference_constructs_from_temporary_v` guards).

- **`optional<T&>` (C++26 / `beman::optional`):** works correctly.  Default `R` is the value
  type (e.g. `int` for `optional<int&>`), so the result is always a fresh value copy.

### Test matrix

| Test file | Coverage |
|-----------|----------|
| `or_construct.test.cpp` | Step 00 smoke: both overloads compile and run |
| `or_construct_behavior.test.cpp` | Return type, engaged/disengaged, value categories, inward conversion, explicit `Ret` |
| `or_construct_construct.test.cpp` | Zero-arg default-construction, multi-arg emplace-style, init-list overload |
| `or_construct_laziness.test.cpp` | Fallback constructed exactly once when disengaged; not constructed when engaged |
| `or_construct_non_nullable_fail.cpp` | Negative compile: non-nullable first arg (pack overload) |
| `or_construct_initlist_non_nullable_fail.cpp` | Negative compile: non-nullable first arg (init-list overload) |
| `or_construct_payload_not_convertible_fail.cpp` | Negative compile: payload not convertible to explicit `Ret` |
| `or_construct_constexpr.test.cpp` | `static_assert`-level constant evaluation: both overloads, `optional`, `expected`, raw pointers, explicit `Ret` |
| `or_construct_optional_ref.test.cpp` | `optional<int&>` and `optional<string&>`: nullable proof, return-type proof (`R = int`), engaged/disengaged, explicit `Ret`, init-list, rebinding |

## `value_or` vs `reference_or` vs `or_invoke` asymmetry

| Function | Return kind | How | Fallback eval |
|----------|-------------|-----|---------------|
| `value_or` | `common_type_t` — always a **value** | `static_cast<R>(*m)` or `static_cast<R>(u)` | eager |
| `reference_or` | `common_reference_t` — usually a **reference** | `static_cast<R>(*m)` or `static_cast<R>(u)` | eager |
| `or_invoke` | `common_type_t` — always a **value** | `static_cast<R>(*m)` or `static_cast<R>(invocable())` | **lazy** (called only when disengaged) |

`reference_or` uses `common_reference_t` (can produce a reference type), while `value_or`
and `or_invoke` use `common_type_t` (always produces a value type).  This is the key
semantic distinction: `reference_or` is for use cases where the caller wants to observe or
mutate through the optional without making a copy; the other two always return by value.

`reference_or` carries two compile-time dangling guards:
```cpp
static_assert(!std::reference_constructs_from_temporary_v<R, U>);
static_assert(!std::reference_constructs_from_temporary_v<R, T&>);
```
These reject type combinations that would bind the returned reference to a temporary.
The negative-compile tests in `ref_or_temp_from_prvalue_fail.cpp` and
`ref_or_rvalue_string_fail.cpp` verify these guards fire correctly.
