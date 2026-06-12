# Handoff 000 — initial seed (from the planner) to the Step 00 agent

**Author:** planning session · **Branch merged:** none yet (clean `main` at the scaffold commit)

## State of `main`
- Fresh Beman scaffold. `main` builds. Existing tests are placeholders:
  - `tests/beman/free_value_or/value_or.test.cpp` — a trivial `CHECK(todo)`.
  - `tests/beman/free_value_or/todo.test.cpp` — placeholder.
  - `tests/beman/free_value_or/CMakeLists.txt` — builds one exe
    `beman.free_value_or.tests.value_or` linking `Catch2::Catch2WithMain`.
- The library under test is header-only: `include/beman/free_value_or/value_or.hpp`,
  namespace `smd::free_value_or` (NOTE: `smd::`, not `beman::`).

## Toolchain facts I observed (verify and expand in Step 00)
- `g++ (Ubuntu) 15.2.0` is on PATH. `clang++` was not found in the planning shell — check.
- Default CMake presets compile at **C++20**. That is too low: the header's `reference_or`
  uses `std::reference_constructs_from_temporary_v` (**C++23**), so it won't even compile
  at C++20. **You must build tests at C++23+.** See PLAN.md §2.
- `std::optional<T&>` is C++26 (P2988) and needs a recent libstdc++. Unverified whether
  GCC 15.2's libstdc++ ships it. **Determine this empirically in Step 00** and record the
  `__cpp_lib_optional` value and whether `optional<int&>` compiles at `-std=c++26`.

## Gotchas to expect
- `nullable` is defined on a `const T` (`requires(const T t){ bool(t); *(t); }`). So a type
  whose `operator bool`/`operator*` are non-const will NOT satisfy it — keep that in mind
  when building model/anti-model types.
- `std::iter_reference_t<T>` is used for the engaged branch's type. It is
  `decltype(*declval<T&>())`. Works for optional/pointers. Confirm it behaves for
  `std::expected` (whose `operator*` exists) — worth a quick static check in Step 00 or 01.
- `value_or`/`or_invoke` return `common_type` (decays to value); `reference_or` returns
  `common_reference` (can be a reference). The dangling static_asserts only live in
  `reference_or`.

## What you (Step 00) should produce
See `steps/step-00-harness.md`. In short: a CMake test scaffold that compiles a sample
translation unit using all three functions at C++23 (proving the toolchain is adequate), a
shared `tests/beman/free_value_or/test_types.hpp` with the model + anti-model types the
later steps will reuse, and a reusable CMake helper/pattern for negative-compile
(WILL_FAIL) tests. Record every toolchain fact you learn in `CHECKLIST.md` and your
handoff — later agents depend on it and start cold.

## What's next after you
Step 01 (concept static_asserts). Leave `test_types.hpp` in a state Step 01 can include
directly.
