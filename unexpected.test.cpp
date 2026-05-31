// beman/expected/unexpected.test.cpp                                 -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/expected/unexpected.hpp>
#include <beman/expected/unexpected.hpp> // ensure idempotent header

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace expt = beman::expected;

// =============================================================================
// [expected.un.general] para 2 — ill-formed instantiation constraints
// (actual ill-formed cases tested via negative compile files)
// =============================================================================

// [expected.un.cons] Constraint 1.3: is_constructible_v<E, Err> must be true
static_assert(std::is_constructible_v<expt::unexpected<int>, int>);
static_assert(std::is_constructible_v<expt::unexpected<std::string>, const char*>);
static_assert(!std::is_constructible_v<expt::unexpected<int>, std::string>);

// [expected.un.cons] Constraint 1.2: the *converting* ctor excludes in_place_t as Err,
// routing it to the in-place constructor instead. Both work:
static_assert(std::is_constructible_v<expt::unexpected<int>, std::in_place_t>); // in-place ctor

// Copy and move constructible
static_assert(std::is_copy_constructible_v<expt::unexpected<int>>);
static_assert(std::is_move_constructible_v<expt::unexpected<int>>);

// [expected.un.swap] Constraint: is_swappable_v<E>
static_assert(std::is_swappable_v<expt::unexpected<int>>);

// [expected.un.obs] error() ref-qualification return types
static_assert(std::is_same_v<decltype(std::declval<expt::unexpected<int>&>().error()), int&>);
static_assert(std::is_same_v<decltype(std::declval<const expt::unexpected<int>&>().error()), const int&>);
static_assert(std::is_same_v<decltype(std::declval<expt::unexpected<int>&&>().error()), int&&>);
static_assert(std::is_same_v<decltype(std::declval<const expt::unexpected<int>&&>().error()), const int&&>);

TEST_CASE("unexpected: construct from int", "[UnexpectedTest]") {
    expt::unexpected<int> u(42);
    CHECK(u.error() == 42);
}

TEST_CASE("unexpected: construct from string", "[UnexpectedTest]") {
    expt::unexpected<std::string> u(std::string("error"));
    CHECK(u.error() == "error");
}

TEST_CASE("unexpected: construct from string literal", "[UnexpectedTest]") {
    expt::unexpected<std::string> u("literal");
    CHECK(u.error() == "literal");
}

TEST_CASE("unexpected: in-place construct string", "[UnexpectedTest]") {
    expt::unexpected<std::string> u(std::in_place, "hello");
    CHECK(u.error() == "hello");
}

TEST_CASE("unexpected: in-place construct vector with initializer_list", "[UnexpectedTest]") {
    expt::unexpected<std::vector<int>> u(std::in_place, std::initializer_list<int>{1, 2, 3});
    CHECK(u.error().size() == 3u);
    CHECK(u.error()[0] == 1);
    CHECK(u.error()[2] == 3);
}

TEST_CASE("unexpected: in-place construct string multi-arg", "[UnexpectedTest]") {
    expt::unexpected<std::string> u(std::in_place, 3u, 'x');
    CHECK(u.error() == "xxx");
}

TEST_CASE("unexpected: copy construct", "[UnexpectedTest]") {
    expt::unexpected<int> a(10);
    expt::unexpected<int> b(a);
    CHECK(b.error() == 10);
}

TEST_CASE("unexpected: move construct", "[UnexpectedTest]") {
    expt::unexpected<std::string> a("moved");
    expt::unexpected<std::string> b(std::move(a));
    CHECK(b.error() == "moved");
}

TEST_CASE("unexpected: error() const lvalue ref", "[UnexpectedTest]") {
    const expt::unexpected<int> u(7);
    const int&                  r = u.error();
    CHECK(r == 7);
}

TEST_CASE("unexpected: error() lvalue ref mutable", "[UnexpectedTest]") {
    expt::unexpected<int> u(7);
    int&                  r = u.error();
    r                       = 99;
    CHECK(u.error() == 99);
}

TEST_CASE("unexpected: error() rvalue ref", "[UnexpectedTest]") {
    expt::unexpected<std::string> u("rval");
    std::string                   s = std::move(u).error();
    CHECK(s == "rval");
}

TEST_CASE("unexpected: error() const rvalue ref", "[UnexpectedTest]") {
    const expt::unexpected<std::string> u("crval");
    std::string                         s = std::move(u).error();
    CHECK(s == "crval");
}

TEST_CASE("unexpected: swap member", "[UnexpectedTest]") {
    expt::unexpected<int> a(1);
    expt::unexpected<int> b(2);
    a.swap(b);
    CHECK(a.error() == 2);
    CHECK(b.error() == 1);
}

TEST_CASE("unexpected: swap friend (ADL)", "[UnexpectedTest]") {
    expt::unexpected<std::string> a("hello");
    expt::unexpected<std::string> b("world");
    swap(a, b);
    CHECK(a.error() == "world");
    CHECK(b.error() == "hello");
}

TEST_CASE("unexpected: equality same type", "[UnexpectedTest]") {
    expt::unexpected<int> a(5);
    expt::unexpected<int> b(5);
    expt::unexpected<int> c(6);
    CHECK(a == b);
    CHECK_FALSE(a == c);
}

TEST_CASE("unexpected: equality different types", "[UnexpectedTest]") {
    expt::unexpected<int>  a(42);
    expt::unexpected<long> b(42L);
    CHECK(a == b);
}

TEST_CASE("unexpected: CTAD from int", "[UnexpectedTest]") {
    expt::unexpected u(42);
    static_assert(std::is_same_v<decltype(u), expt::unexpected<int>>);
    CHECK(u.error() == 42);
}

TEST_CASE("unexpected: CTAD from string", "[UnexpectedTest]") {
    std::string      s("deduced");
    expt::unexpected u(s);
    static_assert(std::is_same_v<decltype(u), expt::unexpected<std::string>>);
    CHECK(u.error() == "deduced");
}

TEST_CASE("unexpected: copy and move constructible", "[UnexpectedTest]") {
    static_assert(std::is_copy_constructible_v<expt::unexpected<int>>);
    static_assert(std::is_move_constructible_v<expt::unexpected<std::string>>);
}

TEST_CASE("unexpected: unexpect_t tag type", "[UnexpectedTest]") {
    static_assert(std::is_same_v<decltype(expt::unexpect), const expt::unexpect_t>);
}

TEST_CASE("unexpected: constexpr basic usage", "[UnexpectedTest]") {
    constexpr expt::unexpected<int> u(123);
    static_assert(u.error() == 123);
}

TEST_CASE("unexpected: inequality operator (synthesized)", "[UnexpectedTest]") {
    expt::unexpected<int> a(1), b(2), c(1);
    CHECK(a != b);
    CHECK_FALSE(a != c);
}

TEST_CASE("unexpected: in-place ilist constraint: is_constructible from ilist", "[UnexpectedTest]") {
    // is_constructible_v<E, initializer_list<U>&, Args...> must hold
    static_assert(
        std::is_constructible_v<expt::unexpected<std::vector<int>>, std::in_place_t, std::initializer_list<int>>);
}
