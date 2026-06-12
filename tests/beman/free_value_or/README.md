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
| `fail_not_nullable.cpp` | Negative compile: calling `value_or` with a non-nullable first arg must fail (`no matching function`) |
| `ref_or_temp_from_prvalue_fail.cpp` | Negative compile: `reference_or` with prvalue fallback that would dangle must fail (`static assertion failed`) |
| `ref_or_rvalue_string_fail.cpp` | Negative compile: `reference_or` with a `string`-from-literal fallback that would dangle must fail |
| `value_or_non_nullable_fail.cpp` | Negative compile: `value_or` with non-nullable first arg |
| `reference_or_non_nullable_fail.cpp` | Negative compile: `reference_or` with non-nullable first arg |
| `or_invoke_non_nullable_fail.cpp` | Negative compile: `or_invoke` with non-nullable first arg |

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
