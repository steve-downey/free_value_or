// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "test_types.hpp"

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

#if FVO_HAS_EXPECTED_REF

using expected_ref = beman::expected::expected<int&, int>;

static_assert(fvo::nullable<expected_ref>);
static_assert(fvo::borrowed_nullable<expected_ref>);
static_assert(std::is_same_v<decltype(fvo::reference_or(std::declval<expected_ref>(), std::declval<int&>())), int&>);

TEST_CASE("reference_or accepts a temporary expected<T&, E>", "[expected_ref][reference_or]") {
    int  value    = 42;
    int  fallback = 0;
    int& result   = fvo::reference_or(expected_ref{value}, fallback);

    CHECK(&result == &value);
}

TEST_CASE("disengaged temporary expected<T&, E> refers to the fallback", "[expected_ref][reference_or]") {
    int  fallback = 7;
    int& result   = fvo::reference_or(expected_ref{beman::expected::unexpected<int>{0}}, fallback);

    CHECK(&result == &fallback);
}

#endif // FVO_HAS_EXPECTED_REF

TEST_CASE("expected_ref tests compiled and feature gate active") {
#if FVO_HAS_EXPECTED_REF
    SUCCEED("FVO_HAS_EXPECTED_REF=1: expected<T&, E> tests ran above");
#else
    SUCCEED("FVO_HAS_EXPECTED_REF=0: expected<T&, E> tests skipped");
#endif
}
