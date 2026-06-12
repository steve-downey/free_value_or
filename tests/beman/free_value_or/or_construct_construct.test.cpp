// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Include-guard idempotency check
#include <beman/free_value_or/value_or.hpp>
#include <beman/free_value_or/value_or.hpp>

#include "test_types.hpp"

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <string>
#include <utility>
#include <vector>

// ==========================================================================
// Return-type static_asserts for emplace/init-list overloads
//
// R = remove_cvref_t<iter_reference_t<T>> when Ret is omitted.
// Pack overload and init-list overload must both yield the payload type.
// ==========================================================================

// Pack, zero args: optional<string> → R = string
static_assert(std::is_same_v<
    decltype(fvo::or_construct(std::declval<std::optional<std::string>&>())),
    std::string>);

// Pack, many args: optional<string> → R = string (arg shape doesn't change R)
static_assert(std::is_same_v<
    decltype(fvo::or_construct(std::declval<std::optional<std::string>&>(),
                               std::declval<std::size_t>(),
                               std::declval<char>())),
    std::string>);

// Pack, pair: optional<pair<int,int>> → R = pair<int,int>
static_assert(std::is_same_v<
    decltype(fvo::or_construct(std::declval<std::optional<std::pair<int, int>>&>(),
                               std::declval<int>(),
                               std::declval<int>())),
    std::pair<int, int>>);

// Init-list: optional<vector<int>> → R = vector<int>
static_assert(std::is_same_v<
    decltype(fvo::or_construct(std::declval<std::optional<std::vector<int>>&>(),
                               std::initializer_list<int>{})),
    std::vector<int>>);

// Explicit Ret with init-list: or_construct<vector<long>>(..., {1L,2L,3L}) → vector<long>
static_assert(std::is_same_v<
    decltype(fvo::or_construct<std::vector<long>>(
        std::declval<std::optional<std::vector<long>>&>(),
        std::initializer_list<long>{})),
    std::vector<long>>);

// ==========================================================================
// Pack overload — construction arity
// ==========================================================================

TEST_CASE("or_construct emplace: zero args default-constructs the payload", "[or_construct]") {
    SECTION("string defaults to empty") {
        std::optional<std::string> m{};
        auto r = fvo::or_construct(m);
        static_assert(std::is_same_v<decltype(r), std::string>);
        CHECK(r == "");
    }
    SECTION("int defaults to 0") {
        std::optional<int> m{};
        auto r = fvo::or_construct(m);
        static_assert(std::is_same_v<decltype(r), int>);
        CHECK(r == 0);
    }
    SECTION("vector<int> defaults to empty") {
        std::optional<std::vector<int>> m{};
        auto r = fvo::or_construct(m);
        static_assert(std::is_same_v<decltype(r), std::vector<int>>);
        CHECK(r.empty());
    }
    SECTION("engaged zero-arg short-circuits to held value") {
        std::optional<std::string> m{"kept"};
        auto r = fvo::or_construct(m);
        CHECK(r == "kept");
    }
}

TEST_CASE("or_construct emplace: one arg constructs the payload", "[or_construct]") {
    SECTION("disengaged: constructs string from literal") {
        std::optional<std::string> m{};
        auto r = fvo::or_construct(m, "hi");
        CHECK(r == "hi");
    }
    SECTION("engaged: short-circuits to held value, ignores arg") {
        std::optional<std::string> m{"held"};
        auto r = fvo::or_construct(m, "ignored");
        CHECK(r == "held");
    }
}

TEST_CASE("or_construct emplace: multiple args (emplace-style)", "[or_construct]") {
    SECTION("string(size_t, char) → disengaged") {
        std::optional<std::string> m{};
        auto r = fvo::or_construct(m, std::size_t{3}, 'x');
        static_assert(std::is_same_v<decltype(r), std::string>);
        CHECK(r == "xxx");
    }
    SECTION("string(size_t, char) → engaged short-circuits") {
        std::optional<std::string> m{"held"};
        auto r = fvo::or_construct(m, std::size_t{3}, 'x');
        CHECK(r == "held");
    }
    SECTION("pair<int,int>(int, int) → disengaged") {
        std::optional<std::pair<int, int>> m{};
        auto r = fvo::or_construct(m, 1, 2);
        static_assert(std::is_same_v<decltype(r), std::pair<int, int>>);
        CHECK(r == std::make_pair(1, 2));
    }
    SECTION("pair<int,int>(int, int) → engaged short-circuits") {
        std::optional<std::pair<int, int>> m{std::in_place, 9, 9};
        auto r = fvo::or_construct(m, 1, 2);
        CHECK(r == std::make_pair(9, 9));
    }
}

TEST_CASE("or_construct emplace: move-in argument forwarded correctly", "[or_construct]") {
    std::optional<std::string> m{};
    std::string src{"move me"};
    auto r = fvo::or_construct(m, std::move(src));
    CHECK(r == "move me");
}

// ==========================================================================
// init-list overload
// ==========================================================================

TEST_CASE("or_construct init-list: plain initializer_list", "[or_construct]") {
    SECTION("vector<int> from {1,2,3} — disengaged") {
        std::optional<std::vector<int>> m{};
        auto r = fvo::or_construct(m, {1, 2, 3});
        static_assert(std::is_same_v<decltype(r), std::vector<int>>);
        CHECK(r == std::vector<int>{1, 2, 3});
    }
    SECTION("string from {'a','b','c'} — disengaged") {
        std::optional<std::string> m{};
        auto r = fvo::or_construct(m, {'a', 'b', 'c'});
        static_assert(std::is_same_v<decltype(r), std::string>);
        CHECK(r == "abc");
    }
    SECTION("vector<int> — engaged short-circuits") {
        std::optional<std::vector<int>> m{std::vector<int>{9}};
        auto r = fvo::or_construct(m, {1, 2, 3});
        CHECK(r == std::vector<int>{9});
    }
}

TEST_CASE("or_construct init-list: initializer_list with trailing args", "[or_construct]") {
    // vector has a (initializer_list<T>, Allocator) constructor.
    // A braced-init-list with a trailing allocator arg routes through the
    // init-list overload: or_construct(m, il, alloc).
    SECTION("vector<int> from {1,2,3} + allocator — disengaged") {
        std::optional<std::vector<int>> m{};
        auto r = fvo::or_construct(m, {1, 2, 3}, std::allocator<int>{});
        static_assert(std::is_same_v<decltype(r), std::vector<int>>);
        CHECK(r == std::vector<int>{1, 2, 3});
    }
    SECTION("vector<int> — engaged short-circuits") {
        std::optional<std::vector<int>> m{std::vector<int>{9}};
        auto r = fvo::or_construct(m, {1, 2, 3}, std::allocator<int>{});
        CHECK(r == std::vector<int>{9});
    }
}

// ==========================================================================
// Explicit Ret with init-list / emplace overloads
// ==========================================================================

TEST_CASE("or_construct emplace: explicit Ret with init-list overload", "[or_construct]") {
    // or_construct<vector<long>>(opt, {1L,2L,3L}) — Ret = vector<long>
    SECTION("disengaged: constructs vector<long> from init-list") {
        std::optional<std::vector<long>> m{};
        auto r = fvo::or_construct<std::vector<long>>(m, {1L, 2L, 3L});
        static_assert(std::is_same_v<decltype(r), std::vector<long>>);
        CHECK(r == std::vector<long>{1L, 2L, 3L});
    }
    SECTION("engaged: static_cast<vector<long>>(*m)") {
        std::optional<std::vector<long>> m{std::vector<long>{9L}};
        auto r = fvo::or_construct<std::vector<long>>(m, {1L, 2L, 3L});
        CHECK(r == std::vector<long>{9L});
    }
}
