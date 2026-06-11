// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "test_types.hpp"

#include <catch2/catch_all.hpp>

TEST_CASE("smoke: value_or with optional<int>", "[smoke]") {
    auto engaged    = NullableFixture<int>::opt_engaged(42);
    auto disengaged = NullableFixture<int>::opt_disengaged();

    CHECK(fvo::value_or(engaged, 0) == 42);
    CHECK(fvo::value_or(disengaged, 0) == 0);
}
