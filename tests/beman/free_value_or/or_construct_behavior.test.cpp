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
#include <string_view>
#include <type_traits>

// ==========================================================================
// Return-type static_asserts
//
// R = remove_cvref_t<iter_reference_t<T>>
// iter_reference_t<T> = decltype(*declval<T&>())
//
// For all optional/expected/pointer types holding int, dereferencing an
// lvalue gives int&, so iter_reference_t = int& and remove_cvref_t<int&> = int.
// Unlike value_or (which uses common_type), or_construct always decays to the
// payload type — the arg is converted *inward* to R, not outward via common_type.
// ==========================================================================

// optional<int> lvalue + int → R = int
static_assert(
    std::is_same_v<decltype(fvo::or_construct(std::declval<std::optional<int>&>(), std::declval<int>())), int>);

// optional<int> rvalue + int → R = int
static_assert(
    std::is_same_v<decltype(fvo::or_construct(std::declval<std::optional<int>>(), std::declval<int>())), int>);

// const optional<int> + int → remove_cvref_t<const int&> = int
static_assert(
    std::is_same_v<decltype(fvo::or_construct(std::declval<const std::optional<int>&>(), std::declval<int>())), int>);

// expected<int,int> + int → R = int
static_assert(
    std::is_same_v<decltype(fvo::or_construct(std::declval<std::expected<int, int>&>(), std::declval<int>())), int>);

// int* + int → R = int
static_assert(std::is_same_v<decltype(fvo::or_construct(std::declval<int*>(), std::declval<int>())), int>);

// shared_ptr<int> + int → R = int
static_assert(
    std::is_same_v<decltype(fvo::or_construct(std::declval<std::shared_ptr<int>&>(), std::declval<int>())), int>);

// unique_ptr<int> rvalue + int → R = int
static_assert(
    std::is_same_v<decltype(fvo::or_construct(std::declval<std::unique_ptr<int>>(), std::declval<int>())), int>);

// Inward conversion: optional<int> + long → R = int (NOT long).
// This contrasts with value_or, which uses common_type and would promote to long.
// or_construct constructs R from the arg, so the arg is converted inward.
static_assert(
    std::is_same_v<decltype(fvo::or_construct(std::declval<std::optional<int>&>(), std::declval<long>())), int>);

// optional<long> + int → R = long (arg int converts inward to long)
static_assert(
    std::is_same_v<decltype(fvo::or_construct(std::declval<std::optional<long>&>(), std::declval<int>())), long>);

// Explicit Ret: or_construct<long>(optional<int>, ...) → R = long
static_assert(
    std::is_same_v<decltype(fvo::or_construct<long>(std::declval<std::optional<int>&>(), std::declval<long>())),
                   long>);

// Explicit Ret: or_construct<std::string>(optional<std::string_view>, ...) → R = std::string
static_assert(std::is_same_v<decltype(fvo::or_construct<std::string>(std::declval<std::optional<std::string_view>&>(),
                                                                     std::declval<const char*>())),
                             std::string>);

// ==========================================================================
// Runtime tests: engaged / disengaged for each nullable type
//
// Engaged path: static_cast<R>(*m) — a fresh R, not an alias/reference.
// Disengaged path: R(std::forward<Args>(args)...) — constructed from arg.
// ==========================================================================

TEST_CASE("or_construct behavior: optional<int> engaged and disengaged", "[or_construct]") {
    std::optional<int> engaged{42};
    std::optional<int> disengaged{};

    CHECK(fvo::or_construct(engaged, 99) == 42);    // returns *m, not arg
    CHECK(fvo::or_construct(disengaged, 99) == 99); // constructs int(99)
}

TEST_CASE("or_construct behavior: expected<int,int> engaged and disengaged", "[or_construct]") {
    auto engaged    = NullableFixture<int>::exp_engaged(42);
    auto disengaged = NullableFixture<int>::exp_disengaged();

    CHECK(fvo::or_construct(engaged, 99) == 42);
    CHECK(fvo::or_construct(disengaged, 99) == 99);
}

TEST_CASE("or_construct behavior: int* engaged and disengaged", "[or_construct]") {
    int  obj        = 42;
    int* engaged    = raw_engaged<int>(obj);
    int* disengaged = raw_disengaged<int>();

    CHECK(fvo::or_construct(engaged, 99) == 42);
    CHECK(fvo::or_construct(disengaged, 99) == 99);
}

TEST_CASE("or_construct behavior: shared_ptr<int> engaged and disengaged", "[or_construct]") {
    auto engaged    = NullableFixture<int>::sptr_engaged(42);
    auto disengaged = NullableFixture<int>::sptr_disengaged();

    CHECK(fvo::or_construct(engaged, 99) == 42);
    CHECK(fvo::or_construct(disengaged, 99) == 99);
}

TEST_CASE("or_construct behavior: unique_ptr<int> engaged and disengaged", "[or_construct]") {
    auto disengaged = NullableFixture<int>::uptr_disengaged();
    CHECK(fvo::or_construct(disengaged, 99) == 99);

    // unique_ptr is move-only; pass as rvalue for the engaged case
    CHECK(fvo::or_construct(NullableFixture<int>::uptr_engaged(42), 99) == 42);
}

// ==========================================================================
// Engaged result is a copy, not a reference
//
// The engaged path is static_cast<R>(*m): it builds a fresh R from *m.
// Mutating the source after the call must not affect the already-returned value.
// (Contrast reference_or, which binds a reference to *m.)
// ==========================================================================

TEST_CASE("or_construct behavior: engaged result is a copy, not a reference", "[or_construct]") {
    std::optional<int> m{42};
    int                result = fvo::or_construct(m, 0); // result = int(42) — a copy
    *m                        = 999;                     // mutate source
    CHECK(result == 42);                                 // copy unaffected
}

// ==========================================================================
// Value categories of m
// ==========================================================================

TEST_CASE("or_construct behavior: value categories of m", "[or_construct]") {
    SECTION("lvalue engaged") {
        std::optional<int> m{42};
        CHECK(fvo::or_construct(m, 0) == 42);
    }
    SECTION("const lvalue engaged") {
        const std::optional<int> m{42};
        CHECK(fvo::or_construct(m, 0) == 42);
    }
    SECTION("rvalue engaged") { CHECK(fvo::or_construct(std::optional<int>{42}, 0) == 42); }
    SECTION("lvalue disengaged") {
        std::optional<int> m{};
        CHECK(fvo::or_construct(m, 99) == 99);
    }
    SECTION("const lvalue disengaged") {
        const std::optional<int> m{};
        CHECK(fvo::or_construct(m, 99) == 99);
    }
    SECTION("rvalue disengaged") { CHECK(fvo::or_construct(std::optional<int>{}, 99) == 99); }
}

// ==========================================================================
// Value categories of the argument
// ==========================================================================

TEST_CASE("or_construct behavior: value categories of the argument", "[or_construct]") {
    std::optional<int> disengaged{};

    SECTION("lvalue arg") {
        int u = 99;
        CHECK(fvo::or_construct(disengaged, u) == 99);
    }
    SECTION("rvalue/temporary arg") { CHECK(fvo::or_construct(disengaged, 99) == 99); }
}

// ==========================================================================
// Inward conversion vs value_or outward common_type promotion
//
// or_construct<default Ret>: R = payload type; arg converts *inward* to R.
// value_or: R = common_type(payload, arg) — may *promote* outward.
//
// optional<int> disengaged + long(99L):
//   value_or  → long (promotes int and long to long)
//   or_construct → int (constructs int from 99L — narrowing construction)
//
// optional<long> disengaged + int(99):
//   value_or  → long (promotes)
//   or_construct → long (constructs long from 99)
// ==========================================================================

TEST_CASE("or_construct behavior: inward conversion (no common_type)", "[or_construct]") {
    SECTION("optional<int> disengaged + long arg → R is int, arg converts inward") {
        std::optional<int> m{};
        auto               result = fvo::or_construct(m, 99L);
        static_assert(std::is_same_v<decltype(result), int>);
        CHECK(result == 99);
    }
    SECTION("optional<int> engaged + long arg → engaged path, R is int") {
        std::optional<int> m{42};
        auto               result = fvo::or_construct(m, 99L);
        static_assert(std::is_same_v<decltype(result), int>);
        CHECK(result == 42);
    }
    SECTION("optional<long> disengaged + int arg → R is long, arg converts inward to long") {
        std::optional<long> m{};
        auto                result = fvo::or_construct(m, 99);
        static_assert(std::is_same_v<decltype(result), long>);
        CHECK(result == 99L);
    }
    SECTION("optional<long> engaged + int arg → R is long") {
        std::optional<long> m{7L};
        auto                result = fvo::or_construct(m, 99);
        static_assert(std::is_same_v<decltype(result), long>);
        CHECK(result == 7L);
    }
}

// ==========================================================================
// Explicit Ret — caller names the result type; payload need only be convertible
// ==========================================================================

TEST_CASE("or_construct behavior: explicit Ret widens payload to long", "[or_construct]") {
    // Engaged: int (*m) converts to long via static_cast<long>
    std::optional<int> engaged{7};
    auto               r1 = fvo::or_construct<long>(engaged, 0L);
    static_assert(std::is_same_v<decltype(r1), long>);
    CHECK(r1 == 7L);

    // Disengaged: constructs long(9L)
    std::optional<int> disengaged{};
    auto               r2 = fvo::or_construct<long>(disengaged, 9L);
    static_assert(std::is_same_v<decltype(r2), long>);
    CHECK(r2 == 9L);
}

TEST_CASE("or_construct behavior: explicit Ret as std::string from string_view payload", "[or_construct]") {
    // Engaged: string_view (*m) converts to std::string
    std::optional<std::string_view> engaged{"held"};
    auto                            r1 = fvo::or_construct<std::string>(engaged, "fb");
    static_assert(std::is_same_v<decltype(r1), std::string>);
    CHECK(r1 == "held");

    // Disengaged: constructs std::string("fb")
    std::optional<std::string_view> disengaged{};
    auto                            r2 = fvo::or_construct<std::string>(disengaged, "fb");
    static_assert(std::is_same_v<decltype(r2), std::string>);
    CHECK(r2 == "fb");
}

TEST_CASE("or_construct behavior: explicit Ret with multi-arg disengaged construction", "[or_construct]") {
    // Disengaged: constructs std::string(3, 'x') → "xxx"
    std::optional<std::string_view> disengaged{};
    auto                            r = fvo::or_construct<std::string>(disengaged, 3, 'x');
    static_assert(std::is_same_v<decltype(r), std::string>);
    CHECK(r == "xxx");
}

TEST_CASE("or_construct behavior: explicit Ret is exactly Ret, not common_type", "[or_construct]") {
    // Return type is long exactly — not a common_type negotiation with the payload.
    std::optional<int> m{};
    static_assert(std::is_same_v<decltype(fvo::or_construct<long>(m, 0L)), long>);
}

// ==========================================================================
// Non-int payload: std::string
// ==========================================================================

TEST_CASE("or_construct behavior: non-int payload (std::string)", "[or_construct]") {
    auto engaged    = NullableFixture<std::string>::opt_engaged("hello");
    auto disengaged = NullableFixture<std::string>::opt_disengaged();

    SECTION("std::string arg") {
        std::string fallback{"world"};
        CHECK(fvo::or_construct(engaged, fallback) == "hello");
        CHECK(fvo::or_construct(disengaged, fallback) == "world");
    }
    SECTION("const char* arg constructs std::string") {
        CHECK(fvo::or_construct(engaged, "world") == "hello");
        CHECK(fvo::or_construct(disengaged, "world") == "world");
    }
}
