// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Include-guard idempotency check
#include <beman/free_value_or/value_or.hpp>
#include <beman/free_value_or/value_or.hpp>

#include "test_types.hpp"

#include <catch2/catch_test_macros.hpp>

#include <initializer_list>
#include <optional>
#include <vector>

namespace {
struct immovable {
    int value;

    explicit immovable(int v) : value(v) {}
    immovable(std::initializer_list<int> il) : value(*il.begin()) {}
    immovable(const immovable&) = delete;
    immovable(immovable&&)      = delete;
};

struct by_value_nullable {
    explicit  operator bool() const { return true; }
    immovable operator*() const { return immovable{42}; }
};
} // namespace

TEST_CASE("or_construct smoke: pack overload, scalar, default Ret") {
    CHECK(fvo::or_construct(std::optional<int>{7}, 0) == 7); // engaged
    CHECK(fvo::or_construct(std::optional<int>{}, 5) == 5);  // disengaged → int(5)
    CHECK(fvo::or_construct(std::optional<int>{}) == 0);     // disengaged, zero args → int{}
}

TEST_CASE("or_construct smoke: pack overload, explicit Ret") {
    // payload int convertible to long
    CHECK(fvo::or_construct<long>(std::optional<int>{7}, 0L) == 7L); // engaged: int → long
    CHECK(fvo::or_construct<long>(std::optional<int>{}, 9L) == 9L);  // disengaged: long(9L)
}

TEST_CASE("or_construct smoke: initializer_list overload") {
    auto v = fvo::or_construct(std::optional<std::vector<int>>{}, {1, 2, 3});
    CHECK(v == std::vector<int>{1, 2, 3}); // disengaged → vector{1,2,3}
}

TEST_CASE("or_construct overloads accept an immovable payload returned by value") {
    auto pack_result      = fvo::or_construct(by_value_nullable{}, 0);
    auto init_list_result = fvo::or_construct(by_value_nullable{}, {0});

    CHECK(pack_result.value == 42);
    CHECK(init_list_result.value == 42);
}
