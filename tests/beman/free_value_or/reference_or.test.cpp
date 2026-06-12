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
// R = common_reference_t<iter_reference_t<T>, U&&>
//
// Contrast with value_or which uses common_type (always a prvalue).
// Here, when both T and U are lvalue references, R is an lvalue reference.
//
// iter_reference_t<optional<int>&> = int& (operator* on lvalue optional)
// U&&  with U=int& (lvalue fallback) = int&
// common_reference_t<int&, int&>   = int&   (lvalue reference — not a copy)
// ==========================================================================

// optional<int>& + int& → common_reference_t<int&, int&> = int&
static_assert(std::is_same_v<
    decltype(fvo::reference_or(std::declval<std::optional<int>&>(), std::declval<int&>())),
    int&>);

// const optional<int>& + const int& → common_reference_t<const int&, const int&> = const int&
static_assert(std::is_same_v<
    decltype(fvo::reference_or(std::declval<const std::optional<int>&>(),
                               std::declval<const int&>())),
    const int&>);

// optional<int>& + const int& → common_reference_t<int&, const int&> = const int&
static_assert(std::is_same_v<
    decltype(fvo::reference_or(std::declval<std::optional<int>&>(), std::declval<const int&>())),
    const int&>);

// const optional<int>& + int& → common_reference_t<const int&, int&> = const int&
static_assert(std::is_same_v<
    decltype(fvo::reference_or(std::declval<const std::optional<int>&>(), std::declval<int&>())),
    const int&>);

// expected<int,int>& + int& → int&
static_assert(std::is_same_v<
    decltype(fvo::reference_or(std::declval<std::expected<int, int>&>(), std::declval<int&>())),
    int&>);

// int* + int& → int&
static_assert(std::is_same_v<
    decltype(fvo::reference_or(std::declval<int*>(), std::declval<int&>())),
    int&>);

// shared_ptr<int>& + int& → int&
static_assert(std::is_same_v<
    decltype(fvo::reference_or(std::declval<std::shared_ptr<int>&>(), std::declval<int&>())),
    int&>);

// string payload: optional<string>& + string& → string&
static_assert(std::is_same_v<
    decltype(fvo::reference_or(std::declval<std::optional<std::string>&>(),
                               std::declval<std::string&>())),
    std::string&>);

// Return is always a reference type (not a prvalue), unlike value_or
static_assert(std::is_reference_v<
    decltype(fvo::reference_or(std::declval<std::optional<int>&>(), std::declval<int&>()))>);

// ==========================================================================
// Case 1 & 2: Reference identity — engaged points into m, disengaged into u
// ==========================================================================

TEST_CASE("reference_or: optional<int> engaged returns reference to contained value",
          "[reference_or]") {
    std::optional<int> m{42};
    int fallback = 0;
    int& r = fvo::reference_or(m, fallback);
    CHECK(&r == &(*m));
    CHECK(r == 42);
}

TEST_CASE("reference_or: optional<int> disengaged returns reference to fallback",
          "[reference_or]") {
    std::optional<int> m{};
    int fallback = 99;
    int& r = fvo::reference_or(m, fallback);
    CHECK(&r == &fallback);
    CHECK(r == 99);
}

// ==========================================================================
// Case 3: Return type is a reference (not a copy)
// ==========================================================================

TEST_CASE("reference_or: return type is a reference, not a prvalue", "[reference_or]") {
    std::optional<int> m{42};
    int fallback = 0;
    static_assert(std::is_reference_v<decltype(fvo::reference_or(m, fallback))>);
    static_assert(std::is_same_v<decltype(fvo::reference_or(m, fallback)), int&>);
}

// ==========================================================================
// Case 4: const propagation
// ==========================================================================

TEST_CASE("reference_or: const optional yields const int& — no mutation through it",
          "[reference_or]") {
    const std::optional<int> m{42};
    const int fallback = 0;
    // R = common_reference_t<const int&, const int&> = const int&
    static_assert(std::is_same_v<decltype(fvo::reference_or(m, fallback)), const int&>);
    const int& r = fvo::reference_or(m, fallback);
    CHECK(r == 42);
    CHECK(&r == &(*m));
}

TEST_CASE("reference_or: optional<int>& + const int& yields const int&", "[reference_or]") {
    std::optional<int> m{42};
    const int fallback = 0;
    // R = common_reference_t<int&, const int&> = const int&
    static_assert(std::is_same_v<decltype(fvo::reference_or(m, fallback)), const int&>);
    const int& r = fvo::reference_or(m, fallback);
    CHECK(r == 42);
}

// ==========================================================================
// Case 5: All confirmed nullable types (int payload, lvalue + lvalue)
// ==========================================================================

TEST_CASE("reference_or: expected<int,int> engaged and disengaged", "[reference_or]") {
    auto engaged    = NullableFixture<int>::exp_engaged(42);
    auto disengaged = NullableFixture<int>::exp_disengaged();
    int fallback    = 99;

    int& r_eng = fvo::reference_or(engaged, fallback);
    CHECK(&r_eng == &(*engaged));
    CHECK(r_eng == 42);

    int& r_dis = fvo::reference_or(disengaged, fallback);
    CHECK(&r_dis == &fallback);
    CHECK(r_dis == 99);
}

TEST_CASE("reference_or: int* engaged and disengaged", "[reference_or]") {
    int  obj        = 42;
    int  fallback   = 99;
    int* engaged    = raw_engaged<int>(obj);
    int* disengaged = raw_disengaged<int>();

    int& r_eng = fvo::reference_or(engaged, fallback);
    CHECK(&r_eng == &obj);
    CHECK(r_eng == 42);

    int& r_dis = fvo::reference_or(disengaged, fallback);
    CHECK(&r_dis == &fallback);
    CHECK(r_dis == 99);
}

TEST_CASE("reference_or: shared_ptr<int> engaged and disengaged", "[reference_or]") {
    auto engaged    = NullableFixture<int>::sptr_engaged(42);
    auto disengaged = NullableFixture<int>::sptr_disengaged();
    int  fallback   = 99;

    int& r_eng = fvo::reference_or(engaged, fallback);
    CHECK(&r_eng == engaged.get());
    CHECK(r_eng == 42);

    int& r_dis = fvo::reference_or(disengaged, fallback);
    CHECK(&r_dis == &fallback);
    CHECK(r_dis == 99);
}

// ==========================================================================
// Case 6: Mutation round-trip — proves true reference, not a copy
// ==========================================================================

TEST_CASE("reference_or: mutation through reference changes contained value (engaged)",
          "[reference_or]") {
    std::optional<int> m{42};
    int fallback = 0;
    int& r = fvo::reference_or(m, fallback);
    r = 99;
    CHECK(*m == 99);
    CHECK(fallback == 0);  // untouched
}

TEST_CASE("reference_or: mutation through reference changes fallback (disengaged)",
          "[reference_or]") {
    std::optional<int> m{};
    int fallback = 0;
    int& r = fvo::reference_or(m, fallback);
    r = 99;
    CHECK(fallback == 99);
    CHECK(!m.has_value());  // optional still disengaged
}

// ==========================================================================
// Non-int payload (std::string)
// ==========================================================================

TEST_CASE("reference_or: optional<string> engaged returns reference to string",
          "[reference_or]") {
    auto        engaged  = NullableFixture<std::string>::opt_engaged("hello");
    std::string fallback = "world";

    std::string& r = fvo::reference_or(engaged, fallback);
    CHECK(&r == &(*engaged));
    CHECK(r == "hello");
}

TEST_CASE("reference_or: optional<string> disengaged returns reference to fallback string",
          "[reference_or]") {
    auto        disengaged = NullableFixture<std::string>::opt_disengaged();
    std::string fallback   = "world";

    std::string& r = fvo::reference_or(disengaged, fallback);
    CHECK(&r == &fallback);
    CHECK(r == "world");
}
