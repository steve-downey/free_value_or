// tests/beman/expected/expected_trivial.test.cpp                       -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Beman-only: triviality of special member functions is implementation quality.
// libstdc++ expected may or may not match these static_asserts.

#include <beman/expected/expected.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <type_traits>

using namespace beman::expected;

// ---------------------------------------------------------------------------
// Primary template: trivial when T and E are trivial
// ---------------------------------------------------------------------------

static_assert(std::is_trivially_copy_constructible_v<expected<int, int>>);
static_assert(std::is_trivially_move_constructible_v<expected<int, int>>);
static_assert(std::is_trivially_copy_assignable_v<expected<int, int>>);
static_assert(std::is_trivially_move_assignable_v<expected<int, int>>);
static_assert(std::is_trivially_destructible_v<expected<int, int>>);

// ---------------------------------------------------------------------------
// Void specialization: trivial when E is trivial
// ---------------------------------------------------------------------------

static_assert(std::is_trivially_copy_constructible_v<expected<void, int>>);
static_assert(std::is_trivially_move_constructible_v<expected<void, int>>);
static_assert(std::is_trivially_copy_assignable_v<expected<void, int>>);
static_assert(std::is_trivially_move_assignable_v<expected<void, int>>);
static_assert(std::is_trivially_destructible_v<expected<void, int>>);

// ---------------------------------------------------------------------------
// Non-trivial when T or E is non-trivial
// ---------------------------------------------------------------------------

static_assert(!std::is_trivially_copy_constructible_v<expected<std::string, int>>);
static_assert(!std::is_trivially_move_constructible_v<expected<std::string, int>>);
static_assert(!std::is_trivially_copy_assignable_v<expected<std::string, int>>);
static_assert(!std::is_trivially_move_assignable_v<expected<std::string, int>>);

static_assert(!std::is_trivially_copy_constructible_v<expected<int, std::string>>);
static_assert(!std::is_trivially_copy_constructible_v<expected<void, std::string>>);

TEST_CASE("trivial SMFs: expected<int,int> is trivially copyable", "[trivial]") {
    expected<int, int> a(42);
    expected<int, int> b = a;
    CHECK(b.has_value());
    CHECK(*b == 42);

    expected<int, int> c = std::move(a);
    CHECK(c.has_value());
    CHECK(*c == 42);

    expected<int, int> d(unexpect, 7);
    b = d;
    CHECK(!b.has_value());
    CHECK(b.error() == 7);
}

TEST_CASE("trivial SMFs: expected<void,int> is trivially copyable", "[trivial]") {
    expected<void, int> a;
    expected<void, int> b = a;
    CHECK(b.has_value());

    expected<void, int> c = std::move(a);
    CHECK(c.has_value());

    expected<void, int> d(unexpect, 7);
    b = d;
    CHECK(!b.has_value());
    CHECK(b.error() == 7);
}
