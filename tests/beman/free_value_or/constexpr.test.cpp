// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "test_types.hpp"

#include <catch2/catch_test_macros.hpp>

#include <expected>
#include <optional>

// ---------------------------------------------------------------------------
// value_or — constant-evaluated proofs
// ---------------------------------------------------------------------------

// engaged: returns held value
static_assert(fvo::value_or(std::optional<int>{7}, 0) == 7);
// disengaged: returns fallback
static_assert(fvo::value_or(std::optional<int>{}, 5) == 5);

// std::expected at C++23 is constexpr-capable
static_assert(fvo::value_or(std::expected<int, int>{42}, 0) == 42);
static_assert(fvo::value_or(std::expected<int, int>{std::unexpected(1)}, 99) == 99);

// raw pointer: constexpr int* pointing at a constexpr object
static constexpr int ce_val = 3;
static_assert(fvo::value_or(&ce_val, 0) == 3);
static_assert(fvo::value_or(static_cast<const int*>(nullptr), 7) == 7);

// ---------------------------------------------------------------------------
// reference_or — constant-evaluated proofs
// ---------------------------------------------------------------------------

static constexpr int ce_a = 10;
static constexpr int ce_b = 20;

// engaged: result references the held int, reads back ce_a value
static_assert(fvo::reference_or(std::optional<int>{ce_a}, ce_b) == ce_a);
// disengaged: result is the fallback reference ce_b
static_assert(fvo::reference_or(std::optional<int>{}, ce_b) == ce_b);

// ---------------------------------------------------------------------------
// or_invoke — constant-evaluated proofs
// ---------------------------------------------------------------------------

// engaged: invocable NOT called, returns held value
static_assert(fvo::or_invoke(std::optional<int>{9}, [] { return 0; }) == 9);
// disengaged: invocable called, returns its result
static_assert(fvo::or_invoke(std::optional<int>{}, [] { return 4; }) == 4);

// std::expected disengaged: invocable supplies the fallback
static_assert(fvo::or_invoke(std::expected<int, int>{std::unexpected(0)}, [] { return 77; }) == 77);

// ---------------------------------------------------------------------------
// Catch2 test case: the real proof is the static_asserts above; this gives
// ctest something to run and report.
// ---------------------------------------------------------------------------
TEST_CASE("constexpr static_asserts compile and hold", "[constexpr]") { SUCCEED(); }
