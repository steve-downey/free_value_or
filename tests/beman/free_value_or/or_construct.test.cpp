// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Include-guard idempotency check
#include <beman/free_value_or/value_or.hpp>
#include <beman/free_value_or/value_or.hpp>

#include "test_types.hpp"

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <vector>

TEST_CASE("or_construct smoke: pack overload, scalar, default Ret") {
    CHECK(fvo::or_construct(std::optional<int>{7}, 0) == 7);   // engaged
    CHECK(fvo::or_construct(std::optional<int>{},  5) == 5);   // disengaged → int(5)
    CHECK(fvo::or_construct(std::optional<int>{})      == 0);  // disengaged, zero args → int{}
}

TEST_CASE("or_construct smoke: pack overload, explicit Ret") {
    // payload int convertible to long
    CHECK(fvo::or_construct<long>(std::optional<int>{7}, 0L) == 7L);  // engaged: int → long
    CHECK(fvo::or_construct<long>(std::optional<int>{},  9L) == 9L);  // disengaged: long(9L)
}

TEST_CASE("or_construct smoke: initializer_list overload") {
    auto v = fvo::or_construct(std::optional<std::vector<int>>{}, {1, 2, 3});
    CHECK(v == std::vector<int>{1, 2, 3});  // disengaged → vector{1,2,3}
}
