// tests/beman/expected/expected_trivial.test.cpp                       -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Beman-only: triviality of special member functions is implementation quality.
// libstdc++ expected may or may not match these checks.
//
// The triviality properties are checked at runtime rather than with
// static_assert so that a violated property is reported by the test run, with
// the responsible type named, instead of stopping the build at the first
// failure and reporting nothing.

#include <beman/expected/expected.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <type_traits>

using namespace beman::expected;

// ---------------------------------------------------------------------------
// Primary template: trivial when T and E are trivial
// ---------------------------------------------------------------------------

TEST_CASE("trivial SMFs: expected<int,int> special members are trivial", "[trivial]") {
    CHECK(std::is_trivially_copy_constructible_v<expected<int, int>>);
    CHECK(std::is_trivially_move_constructible_v<expected<int, int>>);
    CHECK(std::is_trivially_copy_assignable_v<expected<int, int>>);
    CHECK(std::is_trivially_move_assignable_v<expected<int, int>>);
    CHECK(std::is_trivially_destructible_v<expected<int, int>>);
}

// ---------------------------------------------------------------------------
// Void specialization: trivial when E is trivial
// ---------------------------------------------------------------------------

TEST_CASE("trivial SMFs: expected<void,int> special members are trivial", "[trivial]") {
    CHECK(std::is_trivially_copy_constructible_v<expected<void, int>>);
    CHECK(std::is_trivially_move_constructible_v<expected<void, int>>);
    CHECK(std::is_trivially_copy_assignable_v<expected<void, int>>);
    CHECK(std::is_trivially_move_assignable_v<expected<void, int>>);
    CHECK(std::is_trivially_destructible_v<expected<void, int>>);
}

// ---------------------------------------------------------------------------
// Non-trivial when T or E is non-trivial
// ---------------------------------------------------------------------------

TEST_CASE("trivial SMFs: a non-trivial T makes the special members non-trivial", "[trivial]") {
    CHECK_FALSE(std::is_trivially_copy_constructible_v<expected<std::string, int>>);
    CHECK_FALSE(std::is_trivially_move_constructible_v<expected<std::string, int>>);
    CHECK_FALSE(std::is_trivially_copy_assignable_v<expected<std::string, int>>);
    CHECK_FALSE(std::is_trivially_move_assignable_v<expected<std::string, int>>);
}

TEST_CASE("trivial SMFs: a non-trivial E makes the special members non-trivial", "[trivial]") {
    CHECK_FALSE(std::is_trivially_copy_constructible_v<expected<int, std::string>>);
    CHECK_FALSE(std::is_trivially_copy_constructible_v<expected<void, std::string>>);
}

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

// ---------------------------------------------------------------------------
// A trivially movable, non-copyable E: no copy assignment is eligible
// ---------------------------------------------------------------------------

namespace {
struct MoveOnly {
    MoveOnly()                           = default;
    MoveOnly(const MoveOnly&)            = delete;
    MoveOnly(MoveOnly&&)                 = default;
    MoveOnly& operator=(const MoveOnly&) = delete;
    MoveOnly& operator=(MoveOnly&&)      = default;
};
} // namespace

// Neither the trivial nor the non-trivial copy assignment is satisfied for a
// move-only E. The class is still trivially copyable -- [class.prop] counts
// only eligible members -- but clang said otherwise until a deleted candidate
// was added for this case (see the note after the primary template).
TEST_CASE("trivial SMFs: a trivially movable, non-copyable E stays trivially copyable", "[trivial]") {
    int i = 0;
    CHECK(std::is_trivially_copyable_v<expected<int, MoveOnly>>);
    CHECK(std::is_trivially_copyable_v<expected<void, MoveOnly>>);
    CHECK(std::is_trivially_copyable_v<expected<int&, MoveOnly>>);
    CHECK_FALSE(std::is_copy_assignable_v<expected<int, MoveOnly>>);
    CHECK_FALSE(std::is_copy_assignable_v<expected<void, MoveOnly>>);
    CHECK_FALSE(std::is_copy_assignable_v<expected<int&, MoveOnly>>);

    expected<int, MoveOnly> a(42);
    expected<int, MoveOnly> b(unexpect);
    b = std::move(a);
    CHECK(b.has_value());
    CHECK(*b == 42);

    expected<int&, MoveOnly> r(i);
    expected<int&, MoveOnly> s(unexpect);
    s = std::move(r);
    CHECK(&*s == &i);
}
