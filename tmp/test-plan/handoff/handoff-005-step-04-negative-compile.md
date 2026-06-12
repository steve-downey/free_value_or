# Handoff 005 — after Step 04 (negative-compile tests)

**Author:** Step 04 agent · **Date:** 2026-06-12 · **Branch merged:** `test/step-04`

## What I did

Added 5 negative-compile test TUs (EXCLUDE_FROM_ALL, driven only by ctest) via
`fvo_add_compile_fail_test` in `tests/beman/free_value_or/CMakeLists.txt`.

### Family A — dangling rejection (reference_or)

- **`ref_or_temp_from_prvalue_fail.cpp`**: `reference_or(optional<int>&, int{42})`
  — U=int (prvalue), R=`const int&`, first `reference_constructs_from_temporary_v<R,U>` guard fires.
  Regex: `"static assertion failed"`

- **`ref_or_rvalue_string_fail.cpp`**: `reference_or(optional<string>&, string{"x"})`
  — U=string (prvalue), R=`const string&`, same first guard fires.
  Regex: `"static assertion failed"`

### Family B — non-nullable first arg

- **`value_or_non_nullable_fail.cpp`**: `value_or(5, 3)` — int not nullable.
  Regex: `"no matching function for call to"`

- **`reference_or_non_nullable_fail.cpp`**: `reference_or(string_a, string_b)` — string not nullable.
  Regex: `"no matching function for call to"`

- **`or_invoke_non_nullable_fail.cpp`**: `or_invoke(vector<int>{}, lambda)` — vector not nullable.
  Regex: `"no matching function for call to"`

## Build & test commands that actually worked

```bash
cmake -B build -S . -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=./infra/cmake/use-fetch-content.cmake
cmake --build build          # normal targets only — all pass, fail TUs excluded
ctest --test-dir build --output-on-failure   # 31/31 pass
```

## Sanity check performed

Changed `ref_or_temp_from_prvalue` regex to `"XYZZY_SANITY_CHECK_WRONG_REGEX"`,
reconfigured, ran ctest — test correctly FAILED with "Required regular expression not found".
Restored the correct regex; all 31 tests pass.

## Cases confirmed safe on this toolchain (dropped — NOT negative tests)

| Call                                               | Why it compiles safely                                    |
|----------------------------------------------------|-----------------------------------------------------------|
| `reference_or(optional<string>&, "literal")`       | `common_reference_t<string&, const char(&)[N]>` is non-reference (prvalue R → no dangling guard) |
| `reference_or(optional<int>&, long_lvalue)`        | `common_reference_t<int&, long&>` is not a reference to a temporary on GCC 15.2 |

These are documented findings about `common_reference` behaviour — not bugs.

## Regexes used (GCC 15.2 / C++23 — flag for portability)

- `"static assertion failed"` — GCC wording for a `static_assert` failure (has been stable for many versions)
- `"no matching function for call to"` — GCC concept-constraint rejection wording (stable)

Both are simple and unlikely to break across minor GCC versions; they are not tied to trait names or
file paths so they remain portable across toolchains too.

## Issues found

None. No header bugs.

## State of `main`

31/31 tests pass from the main checkout after the merge. All prior tests still green.

## What the next agent (Step 05) should know

- **Step 05** is `or_invoke` results + laziness — `steps/step-05-or_invoke.md`.
- Use `fvo_add_test(name source)` for the positive runtime tests.
- `or_invoke` is in `value_or.hpp`; signature mirrors `value_or` but calls `invocable()` lazily.
- The CHECKLIST.md next-up box is Step 05.
