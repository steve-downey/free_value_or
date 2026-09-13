// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/expected/expected.hpp>
#include <beman/free_value_or/value_or.hpp>

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

template <class T, class E>
inline constexpr bool
    smd::free_value_or::enable_borrowed_nullable<beman::expected::expected<T&, E>> = true;

using expected_ref = beman::expected::expected<int&, int>;

static_assert(smd::free_value_or::nullable<expected_ref>);
static_assert(smd::free_value_or::borrowed_nullable<expected_ref>);
static_assert(std::is_same_v<decltype(smd::free_value_or::reference_or(
                                 std::declval<expected_ref>(), std::declval<int&>())),
                             int&>);

TEST_CASE("reference_or accepts a temporary expected<T&, E>", "[expected_ref][reference_or]") {
    int  value    = 42;
    int  fallback = 0;
    int& result   = smd::free_value_or::reference_or(expected_ref{value}, fallback);

    CHECK(&result == &value);
}

TEST_CASE("disengaged temporary expected<T&, E> refers to the fallback", "[expected_ref][reference_or]") {
    int  fallback = 7;
    int& result   = smd::free_value_or::reference_or(
        expected_ref{beman::expected::unexpected<int>{0}}, fallback);

    CHECK(&result == &fallback);
}
