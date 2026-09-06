// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Include-guard idempotency check
#include <beman/free_value_or/value_or.hpp>
#include <beman/free_value_or/value_or.hpp>

#include "test_types.hpp"

#include <catch2/catch_test_macros.hpp>

#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>

// ==========================================================================
// Return-type static_asserts
//
// R = common_type_t<iter_reference_t<T>, U&&>
// iter_reference_t<T> = decltype(*declval<T&>())
//
// For all optional/expected/pointer types holding int, dereferencing an
// lvalue gives int&, so iter_reference_t = int&.  common_type strips
// references, so common_type_t<int&, int&&> = int (prvalue).
// ==========================================================================

// optional<int> lvalue + int → iter_ref=int&, U&&=int&& → common_type<int,int> = int
static_assert(std::is_same_v<decltype(fvo::value_or(std::declval<std::optional<int>&>(), std::declval<int>())), int>);

// optional<int> rvalue + int → same (iter_reference_t still uses T& = optional<int>&)
static_assert(std::is_same_v<decltype(fvo::value_or(std::declval<std::optional<int>>(), std::declval<int>())), int>);

// const optional<int> + int → iter_ref=const int&, common_type<const int,int> = int
static_assert(
    std::is_same_v<decltype(fvo::value_or(std::declval<const std::optional<int>&>(), std::declval<int>())), int>);

#if FVO_HAS_STD_EXPECTED
// expected<int,int> + int → iter_ref=int& → int
static_assert(
    std::is_same_v<decltype(fvo::value_or(std::declval<std::expected<int, int>&>(), std::declval<int>())), int>);
#endif // FVO_HAS_STD_EXPECTED

// int* + int → iter_ref(*ptr) = int& → int
static_assert(std::is_same_v<decltype(fvo::value_or(std::declval<int*>(), std::declval<int>())), int>);

// shared_ptr<int> + int → int
static_assert(
    std::is_same_v<decltype(fvo::value_or(std::declval<std::shared_ptr<int>&>(), std::declval<int>())), int>);

// unique_ptr<int> rvalue + int → int
static_assert(std::is_same_v<decltype(fvo::value_or(std::declval<std::unique_ptr<int>>(), std::declval<int>())), int>);

// Type mismatch: optional<int> + long → common_type<int,long> = long
static_assert(
    std::is_same_v<decltype(fvo::value_or(std::declval<std::optional<int>&>(), std::declval<long>())), long>);

// Type mismatch: optional<int> + double → common_type<int,double> = double
static_assert(
    std::is_same_v<decltype(fvo::value_or(std::declval<std::optional<int>&>(), std::declval<double>())), double>);

// String payload: optional<string> + string → string
static_assert(
    std::is_same_v<decltype(fvo::value_or(std::declval<std::optional<std::string>&>(), std::declval<std::string>())),
                   std::string>);

// ==========================================================================
// Runtime tests: engaged / disengaged for each nullable type
// ==========================================================================

TEST_CASE("value_or: optional<int> engaged and disengaged", "[value_or]") {
    auto engaged    = NullableFixture<int>::opt_engaged(42);
    auto disengaged = NullableFixture<int>::opt_disengaged();

    CHECK(fvo::value_or(engaged, 0) == 42);
    CHECK(fvo::value_or(disengaged, 0) == 0);
}

#if FVO_HAS_STD_EXPECTED
TEST_CASE("value_or: expected<int,int> engaged and disengaged", "[value_or]") {
    auto engaged    = NullableFixture<int>::exp_engaged(42);
    auto disengaged = NullableFixture<int>::exp_disengaged();

    CHECK(fvo::value_or(engaged, 0) == 42);
    CHECK(fvo::value_or(disengaged, 0) == 0);
}
#endif // FVO_HAS_STD_EXPECTED

TEST_CASE("value_or: int* engaged and disengaged", "[value_or]") {
    int  obj        = 42;
    int* engaged    = raw_engaged<int>(obj);
    int* disengaged = raw_disengaged<int>();

    CHECK(fvo::value_or(engaged, 0) == 42);
    CHECK(fvo::value_or(disengaged, 0) == 0);
}

TEST_CASE("value_or: shared_ptr<int> engaged and disengaged", "[value_or]") {
    auto engaged    = NullableFixture<int>::sptr_engaged(42);
    auto disengaged = NullableFixture<int>::sptr_disengaged();

    CHECK(fvo::value_or(engaged, 0) == 42);
    CHECK(fvo::value_or(disengaged, 0) == 0);
}

TEST_CASE("value_or: unique_ptr<int> engaged and disengaged", "[value_or]") {
    auto disengaged = NullableFixture<int>::uptr_disengaged();
    CHECK(fvo::value_or(disengaged, 0) == 0);

    // unique_ptr is move-only; pass as rvalue for the engaged case
    CHECK(fvo::value_or(NullableFixture<int>::uptr_engaged(42), 0) == 42);
}

// ==========================================================================
// Value categories of m
// ==========================================================================

TEST_CASE("value_or: value categories of m", "[value_or]") {
    SECTION("lvalue engaged") {
        std::optional<int> m{42};
        CHECK(fvo::value_or(m, 0) == 42);
    }
    SECTION("const lvalue engaged") {
        const std::optional<int> m{42};
        CHECK(fvo::value_or(m, 0) == 42);
    }
    SECTION("rvalue engaged") { CHECK(fvo::value_or(std::optional<int>{42}, 0) == 42); }
    SECTION("lvalue disengaged") {
        std::optional<int> m{};
        CHECK(fvo::value_or(m, 99) == 99);
    }
    SECTION("const lvalue disengaged") {
        const std::optional<int> m{};
        CHECK(fvo::value_or(m, 99) == 99);
    }
    SECTION("rvalue disengaged") { CHECK(fvo::value_or(std::optional<int>{}, 99) == 99); }
}

// ==========================================================================
// Value categories of fallback u
// ==========================================================================

TEST_CASE("value_or: value categories of fallback u", "[value_or]") {
    std::optional<int> disengaged{};

    SECTION("lvalue fallback") {
        int u = 99;
        CHECK(fvo::value_or(disengaged, u) == 99);
    }
    SECTION("rvalue/temporary fallback") { CHECK(fvo::value_or(disengaged, 99) == 99); }
}

// ==========================================================================
// Type mismatch — common_type promotion
// ==========================================================================

TEST_CASE("value_or: type mismatch - common_type promotion", "[value_or]") {
    std::optional<int> engaged{42};
    std::optional<int> disengaged{};

    SECTION("int + long → long") {
        long fallback = 99L;
        CHECK(fvo::value_or(engaged, fallback) == 42L);
        CHECK(fvo::value_or(disengaged, fallback) == 99L);
    }
    SECTION("int + double → double") {
        double fallback = 99.0;
        CHECK(fvo::value_or(engaged, fallback) == 42.0);
        CHECK(fvo::value_or(disengaged, fallback) == 99.0);
    }
}

// ==========================================================================
// Non-int payload (std::string)
// ==========================================================================

TEST_CASE("value_or: non-int payload (std::string)", "[value_or]") {
    auto        engaged    = NullableFixture<std::string>::opt_engaged("hello");
    auto        disengaged = NullableFixture<std::string>::opt_disengaged();
    std::string fallback{"world"};

    CHECK(fvo::value_or(engaged, fallback) == "hello");
    CHECK(fvo::value_or(disengaged, fallback) == "world");
}

// ==========================================================================
// Eagerness: fallback is always evaluated (it is a by-value argument)
// ==========================================================================

TEST_CASE("value_or: fallback is evaluated eagerly", "[value_or]") {
    // value_or takes fallback by value; the argument expression is always
    // fully evaluated before the function body runs, even when m is engaged.
    int  count = 0;
    auto make  = [&] {
        ++count;
        return 0;
    };

    std::optional<int> engaged{42};
    std::optional<int> disengaged{};

    fvo::value_or(engaged, make()); // engaged — fallback still constructed
    CHECK(count == 1);

    fvo::value_or(disengaged, make()); // disengaged — fallback constructed too
    CHECK(count == 2);
}
