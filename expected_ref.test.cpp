// tests/beman/expected/expected_ref.test.cpp                         -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/expected/expected.hpp>
#include <beman/expected/expected.hpp>

#include <catch2/catch_test_macros.hpp>

#include <beman/expected/testing/type_name.hpp>

#include "testing/types.hpp"

#include <string>
#include <type_traits>

using namespace beman::expected;

using beman::expected::testing::type_name;

// =============================================================================
// Type-level properties
//
// These are checked at runtime rather than with static_assert so that a
// violated property is reported by the test run, with the responsible type
// named, instead of stopping the build at the first failure and reporting
// nothing. Type identity is compared through `type_name`, whose comparison is
// `std::is_same_v` — exactly as strict as the static_assert it replaces — so
// a mismatch prints what was deduced next to what was wanted rather than the
// bare word `false`.
// =============================================================================

TEST_CASE("expected<T&>: special member availability", "[expected_ref]") {
    using expected_t = expected<int&, int>;
    CHECK((std::is_constructible_v<expected_t, int&>));
    CHECK_FALSE(std::is_default_constructible_v<expected_t>);
    CHECK(std::is_copy_constructible_v<expected_t>);
    CHECK(std::is_move_constructible_v<expected_t>);
}

TEST_CASE("expected<T&>: observer return types", "[expected_ref]") {
    using expected_t = expected<int&, int>;
    CHECK(type_name<decltype(std::declval<expected_t>().operator->())>() == type_name<int*>());
    CHECK(type_name<decltype(*std::declval<expected_t>())>() == type_name<int&>());
    CHECK(type_name<decltype(std::declval<expected_t>().value())>() == type_name<int&>());
}

TEST_CASE("expected<T&>: const expected<T&, E> still returns T* / T& (shallow const)", "[expected_ref]") {
    using expected_t = expected<int&, int>;
    CHECK(type_name<decltype(std::declval<const expected_t>().operator->())>() == type_name<int*>());
    CHECK(type_name<decltype(*std::declval<const expected_t>())>() == type_name<int&>());
}

TEST_CASE("expected<T&>: triviality when E is trivial", "[expected_ref]") {
    // When E is trivial, copy/move/assign/destroy should be trivial
    using expected_t = expected<int&, int>;
    CHECK(std::is_trivially_copy_constructible_v<expected_t>);
    CHECK(std::is_trivially_move_constructible_v<expected_t>);
    CHECK(std::is_trivially_copy_assignable_v<expected_t>);
    CHECK(std::is_trivially_move_assignable_v<expected_t>);
    CHECK(std::is_trivially_destructible_v<expected_t>);
}

TEST_CASE("expected<T&>: non-trivial E is still constructible, but not trivially", "[expected_ref]") {
    using expected_t = expected<int&, std::string>;
    CHECK(std::is_copy_constructible_v<expected_t>);
    CHECK(std::is_move_constructible_v<expected_t>);
    CHECK_FALSE(std::is_trivially_copy_constructible_v<expected_t>);
    CHECK_FALSE(std::is_trivially_move_constructible_v<expected_t>);
    CHECK_FALSE(std::is_trivially_destructible_v<expected_t>);
}

// =============================================================================
// Construction
// =============================================================================

TEST_CASE("expected<T&>: construct from lvalue reference", "[expected_ref]") {
    int                 x = 42;
    expected<int&, int> e(x);
    REQUIRE(e.has_value());
    CHECK(&*e == &x);
    CHECK(*e == 42);
}

TEST_CASE("expected<T&>: construct from unexpected", "[expected_ref]") {
    expected<int&, int> e = unexpected(7);
    REQUIRE(!e.has_value());
    CHECK(e.error() == 7);
}

TEST_CASE("expected<T&>: construct from unexpect_t in-place error", "[expected_ref]") {
    expected<int&, std::string> e(unexpect, "err");
    REQUIRE(!e.has_value());
    CHECK(e.error() == "err");
}

TEST_CASE("expected<T&>: copy construct (copies pointer)", "[expected_ref]") {
    int                 x = 1;
    expected<int&, int> a(x);
    expected<int&, int> b = a;
    REQUIRE(b.has_value());
    CHECK(&*b == &x);
}

TEST_CASE("expected<T&>: move construct (copies pointer)", "[expected_ref]") {
    int                 x = 2;
    expected<int&, int> a(x);
    expected<int&, int> b = std::move(a);
    REQUIRE(b.has_value());
    CHECK(&*b == &x);
}

TEST_CASE("expected<T&>: copy construct error state", "[expected_ref]") {
    expected<int&, std::string> a(unexpect, "oops");
    expected<int&, std::string> b = a;
    REQUIRE(!b.has_value());
    CHECK(b.error() == "oops");
}

TEST_CASE("expected<T&>: copy construct value state (non-trivial E)", "[expected_ref]") {
    int                         x = 42;
    expected<int&, std::string> a(x);
    expected<int&, std::string> b = a;
    REQUIRE(b.has_value());
    CHECK(&*b == &x);
}

TEST_CASE("expected<T&>: construct from derived expected<U&, G>", "[expected_ref]") {
    struct Base {
        virtual ~Base() = default;
        int v;
    };
    struct Derived : Base {
        Derived(int i) { v = i; }
    };

    Derived                 d{99};
    expected<Derived&, int> src(d);
    expected<Base&, int>    dst = src;
    REQUIRE(dst.has_value());
    CHECK(dst->v == 99);
    CHECK(&*dst == static_cast<Base*>(&d));
}

// =============================================================================
// Rebind semantics on assignment
// =============================================================================

TEST_CASE("expected<T&>: rebind reference on assignment from lvalue", "[expected_ref]") {
    int                 x = 1, y = 2;
    expected<int&, int> e(x);
    e = y;
    CHECK(&*e == &y);
    CHECK(*e == 2);
    CHECK(x == 1);
}

TEST_CASE("expected<T&>: rebind does NOT assign through reference", "[expected_ref]") {
    int                 x = 100, y = 200;
    expected<int&, int> e(x);
    e = y;
    CHECK(x == 100);
    CHECK(*e == 200);
}

TEST_CASE("expected<T&>: assign from unexpected transitions to error state", "[expected_ref]") {
    int                 x = 5;
    expected<int&, int> e(x);
    e = unexpected(99);
    REQUIRE(!e.has_value());
    CHECK(e.error() == 99);
    CHECK(x == 5);
}

TEST_CASE("expected<T&>: assign lvalue rebinds from error state", "[expected_ref]") {
    int                 x = 7;
    expected<int&, int> e = unexpected(1);
    e                     = x;
    REQUIRE(e.has_value());
    CHECK(&*e == &x);
}

TEST_CASE("expected<T&>: copy assignment value-value rebinds", "[expected_ref]") {
    int                 x = 1, y = 2;
    expected<int&, int> a(x), b(y);
    a = b;
    CHECK(&*a == &y);
    CHECK(x == 1);
}

TEST_CASE("expected<T&>: copy assignment error-error copies error", "[expected_ref]") {
    expected<int&, int> a = unexpected(1);
    expected<int&, int> b = unexpected(2);
    a                     = b;
    CHECK(a.error() == 2);
}

TEST_CASE("expected<T&>: copy assignment value-to-error", "[expected_ref]") {
    int                 x = 5;
    expected<int&, int> a(x);
    expected<int&, int> b = unexpected(42);
    a                     = b;
    REQUIRE(!a.has_value());
    CHECK(a.error() == 42);
}

TEST_CASE("expected<T&>: copy assignment error-to-value", "[expected_ref]") {
    int                 x = 5;
    expected<int&, int> a = unexpected(42);
    expected<int&, int> b(x);
    a = b;
    REQUIRE(a.has_value());
    CHECK(&*a == &x);
}

TEST_CASE("expected<T&>: move assignment value-to-value (non-trivial E)", "[expected_ref]") {
    int                         x1 = 1, x2 = 2;
    expected<int&, std::string> a(x1), b(x2);
    a = std::move(b);
    REQUIRE(a.has_value());
    CHECK(&*a == &x2);
}

TEST_CASE("expected<T&>: move assignment value-to-error (non-trivial E)", "[expected_ref]") {
    int                         x = 5;
    expected<int&, std::string> a(x);
    expected<int&, std::string> b(unexpect, "moved-err");
    a = std::move(b);
    REQUIRE(!a.has_value());
    CHECK(a.error() == "moved-err");
}

TEST_CASE("expected<T&>: move assignment error-to-value (non-trivial E)", "[expected_ref]") {
    int                         x = 5;
    expected<int&, std::string> a(unexpect, "old-err");
    expected<int&, std::string> b(x);
    a = std::move(b);
    REQUIRE(a.has_value());
    CHECK(&*a == &x);
}

TEST_CASE("expected<T&>: assign from unexpected&& when already has error", "[expected_ref]") {
    expected<int&, int> e = unexpected(1);
    e                     = unexpected(2);
    REQUIRE(!e.has_value());
    CHECK(e.error() == 2);
}

// =============================================================================
// Shallow const
// =============================================================================

TEST_CASE("expected<T&>: shallow const allows mutation of referent", "[expected_ref]") {
    int                       x = 10;
    const expected<int&, int> e(x);
    *e = 20;
    CHECK(x == 20);
}

TEST_CASE("expected<T&>: operator-> on const returns T*", "[expected_ref]") {
    int                       x = 5;
    const expected<int&, int> e(x);
    CHECK(type_name<decltype(e.operator->())>() == type_name<int*>());
    *e.operator->() = 99;
    CHECK(x == 99);
}

// =============================================================================
// Observers
// =============================================================================

TEST_CASE("expected<T&>: operator* returns T&", "[expected_ref]") {
    int                 x = 42;
    expected<int&, int> e(x);
    CHECK(type_name<decltype(*e)>() == type_name<int&>());
    *e = 99;
    CHECK(x == 99);
}

TEST_CASE("expected<T&>: operator-> returns T*", "[expected_ref]") {
    struct S {
        int v;
    };
    S                 s{7};
    expected<S&, int> e(s);
    CHECK(e->v == 7);
    e->v = 99;
    CHECK(s.v == 99);
}

TEST_CASE("expected<T&>: value() returns T& or throws", "[expected_ref]") {
    int                 x = 1;
    expected<int&, int> e(x);
    CHECK(type_name<decltype(e.value())>() == type_name<int&>());
    CHECK(e.value() == 1);
    e.value() = 2;
    CHECK(x == 2);
}

TEST_CASE("expected<T&>: value() throws bad_expected_access on error", "[expected_ref]") {
    expected<int&, int> e = unexpected(5);
    REQUIRE_THROWS_AS(e.value(), beman::expected::bad_expected_access<int>);
}

TEST_CASE("expected<T&>: error() returns error", "[expected_ref]") {
    expected<int&, int> e = unexpected(42);
    CHECK(e.error() == 42);
}

TEST_CASE("expected<T&>: value_or returns referred value when has value", "[expected_ref]") {
    int                 x = 42;
    expected<int&, int> e(x);
    int                 result = e.value_or(0);
    CHECK(result == 42);
}

TEST_CASE("expected<T&>: value_or returns fallback when has error", "[expected_ref]") {
    expected<int&, int> e      = unexpected(0);
    int                 result = e.value_or(99);
    CHECK(result == 99);
}

TEST_CASE("expected<T&>: error_or returns error when has error", "[expected_ref]") {
    expected<int&, int> e = unexpected(42);
    CHECK(e.error_or(0) == 42);
}

TEST_CASE("expected<T&>: error_or returns default when has value", "[expected_ref]") {
    int                 x = 5;
    expected<int&, int> e(x);
    CHECK(e.error_or(99) == 99);
}

TEST_CASE("expected<T&>: bool conversion", "[expected_ref]") {
    int                 x = 1;
    expected<int&, int> val(x);
    expected<int&, int> err = unexpected(0);
    CHECK(static_cast<bool>(val));
    CHECK(!static_cast<bool>(err));
}

// =============================================================================
// emplace
// =============================================================================

TEST_CASE("expected<T&>: emplace rebinds from value", "[expected_ref]") {
    int                 x = 1, y = 2;
    expected<int&, int> e(x);
    e.emplace(y);
    CHECK(&*e == &y);
    CHECK(x == 1);
}

TEST_CASE("expected<T&>: emplace rebinds from error", "[expected_ref]") {
    int                 x = 5;
    expected<int&, int> e = unexpected(42);
    e.emplace(x);
    REQUIRE(e.has_value());
    CHECK(&*e == &x);
}

// =============================================================================
// Swap
// =============================================================================

TEST_CASE("expected<T&>: swap value-value rebinds pointers", "[expected_ref]") {
    int                 x = 1, y = 2;
    expected<int&, int> a(x), b(y);
    a.swap(b);
    CHECK(&*a == &y);
    CHECK(&*b == &x);
    CHECK(x == 1);
    CHECK(y == 2);
}

TEST_CASE("expected<T&>: swap value-error", "[expected_ref]") {
    int                 x = 1;
    expected<int&, int> a(x), b(unexpect, 99);
    a.swap(b);
    REQUIRE(!a.has_value());
    REQUIRE(b.has_value());
    CHECK(a.error() == 99);
    CHECK(&*b == &x);
}

TEST_CASE("expected<T&>: swap error-error", "[expected_ref]") {
    expected<int&, int> a(unexpect, 1), b(unexpect, 2);
    a.swap(b);
    CHECK(a.error() == 2);
    CHECK(b.error() == 1);
}

TEST_CASE("expected<T&>: friend swap", "[expected_ref]") {
    int                 x = 10, y = 20;
    expected<int&, int> a(x), b(y);
    swap(a, b);
    CHECK(&*a == &y);
    CHECK(&*b == &x);
}

// =============================================================================
// Equality
// =============================================================================

TEST_CASE("expected<T&>: equality with expected<T2&, E2>", "[expected_ref]") {
    int                 x = 5, y = 5, z = 6;
    expected<int&, int> a(x), b(y), c(z);
    CHECK(a == b);
    CHECK(!(a == c));
}

TEST_CASE("expected<T&>: equality with value type", "[expected_ref]") {
    int                 x = 42;
    expected<int&, int> e(x);
    CHECK(e == 42);
    CHECK(!(e == 99));
}

TEST_CASE("expected<T&>: equality with unexpected", "[expected_ref]") {
    expected<int&, int> e = unexpected(7);
    CHECK(e == unexpected(7));
    CHECK(!(e == unexpected(8)));
}

TEST_CASE("expected<T&>: inequality value vs error", "[expected_ref]") {
    int                 x = 5;
    expected<int&, int> val(x);
    expected<int&, int> err = unexpected(5);
    CHECK(!(val == err));
}

// =============================================================================
// Monadic operations
// =============================================================================

TEST_CASE("expected<T&>: and_then passes T& to callable", "[expected_ref]") {
    int                 x = 5;
    expected<int&, int> e(x);
    auto                r = e.and_then([](int& v) -> expected<int, int> { return v * 2; });
    REQUIRE(r.has_value());
    CHECK(*r == 10);
}

TEST_CASE("expected<T&>: and_then on error forwards error", "[expected_ref]") {
    expected<int&, int> e = unexpected(42);
    auto                r = e.and_then([](int& v) -> expected<int, int> { return v * 2; });
    REQUIRE(!r.has_value());
    CHECK(r.error() == 42);
}

TEST_CASE("expected<T&>: transform passes T& to callable", "[expected_ref]") {
    int                 x = 3;
    expected<int&, int> e(x);
    auto                r = e.transform([](int& v) { return v + 1; });
    REQUIRE(r.has_value());
    CHECK(*r == 4);
}

TEST_CASE("expected<T&>: transform on error forwards error", "[expected_ref]") {
    expected<int&, int> e = unexpected(99);
    auto                r = e.transform([](int& v) { return v + 1; });
    REQUIRE(!r.has_value());
    CHECK(r.error() == 99);
}

TEST_CASE("expected<T&>: or_else passes error to callable", "[expected_ref]") {
    expected<int&, int> e = unexpected(99);
    int                 x = 0;
    auto                r = e.or_else([&](int v) -> expected<int&, int> {
        x = v;
        return unexpected(v);
    });
    CHECK(x == 99);
    REQUIRE(!r.has_value());
    CHECK(r.error() == 99);
}

TEST_CASE("expected<T&>: or_else on value returns value", "[expected_ref]") {
    int                 x = 42;
    expected<int&, int> e(x);
    auto                r = e.or_else([](int) -> expected<int&, int> { return unexpected(0); });
    REQUIRE(r.has_value());
    CHECK(*r == 42);
}

TEST_CASE("expected<T&>: transform_error transforms error", "[expected_ref]") {
    expected<int&, int> e = unexpected(5);
    auto                r = e.transform_error([](int v) -> std::string { return std::to_string(v); });
    REQUIRE(!r.has_value());
    CHECK(r.error() == "5");
}

TEST_CASE("expected<T&>: transform_error on value preserves value", "[expected_ref]") {
    int                 x = 42;
    expected<int&, int> e(x);
    auto                r = e.transform_error([](int v) -> std::string { return std::to_string(v); });
    REQUIRE(r.has_value());
    CHECK(*r == 42);
    CHECK(&*r == &x);
}

TEST_CASE("expected<T&>: transform to void", "[expected_ref]") {
    int                 x   = 5;
    int                 out = 0;
    expected<int&, int> e(x);
    auto                r = e.transform([&](int& v) { out = v; });
    REQUIRE(r.has_value());
    CHECK(out == 5);
}

TEST_CASE("expected<T&>: transform to void on error - propagates", "[expected_ref]") {
    expected<int&, int> e(unexpect, 5);
    bool                called = false;
    auto                r      = e.transform([&](int&) { called = true; });
    CHECK(!called);
    REQUIRE(!r.has_value());
    CHECK(r.error() == 5);
}

TEST_CASE("expected<T&>: const transform to void on error - propagates", "[expected_ref]") {
    const expected<int&, int> e(unexpect, 5);
    bool                      called = false;
    auto                      r      = e.transform([&](int&) { called = true; });
    CHECK(!called);
    REQUIRE(!r.has_value());
    CHECK(r.error() == 5);
}

// =============================================================================
// const-qualified monadic operations
// =============================================================================

TEST_CASE("expected<T&>: const and_then", "[expected_ref]") {
    int                       x = 10;
    const expected<int&, int> e(x);
    auto                      r = e.and_then([](int& v) -> expected<int, int> { return v + 1; });
    REQUIRE(r.has_value());
    CHECK(*r == 11);
}

TEST_CASE("expected<T&>: const transform", "[expected_ref]") {
    int                       x = 3;
    const expected<int&, int> e(x);
    auto                      r = e.transform([](int& v) { return v * 3; });
    REQUIRE(r.has_value());
    CHECK(*r == 9);
}

TEST_CASE("expected<T&>: const transform on error - propagates", "[expected_ref]") {
    const expected<int&, int> e(unexpect, 5);
    bool                      called = false;
    auto                      r      = e.transform([&](int&) {
        called = true;
        return 0;
    });
    CHECK(!called);
    REQUIRE(!r.has_value());
    CHECK(r.error() == 5);
}

TEST_CASE("expected<T&>: const transform to void - calls F", "[expected_ref]") {
    int                       x   = 3;
    int                       out = 0;
    const expected<int&, int> e(x);
    auto                      r = e.transform([&](int& v) { out = v; });
    REQUIRE(r.has_value());
    CHECK(out == 3);
}

// =============================================================================
// rvalue-qualified monadic operations
// =============================================================================

TEST_CASE("expected<T&>: rvalue and_then", "[expected_ref]") {
    int                 x = 5;
    expected<int&, int> e(x);
    auto                r = std::move(e).and_then([](int& v) -> expected<int, int> { return v * 3; });
    REQUIRE(r.has_value());
    CHECK(*r == 15);
}

TEST_CASE("expected<T&>: rvalue transform_error moves error", "[expected_ref]") {
    expected<int&, std::string> e(unexpect, "hello");
    auto r = std::move(e).transform_error([](std::string&& s) -> std::string { return s + " world"; });
    REQUIRE(!r.has_value());
    CHECK(r.error() == "hello world");
}

// =============================================================================
// Additional coverage tests: T& rvalue/const-rvalue monadic and traced paths
// =============================================================================
using namespace beman::expected::testing;

// --- Monadic: and_then rvalue/const-rvalue on ERROR state ---
TEST_CASE("and_then rvalue on error short-circuits", "[expected_ref]") {
    expected<int&, std::string> e(unexpect, "err");
    auto                        r = std::move(e).and_then([](int& v) -> expected<int, std::string> { return v; });
    REQUIRE(!r.has_value());
    CHECK(r.error() == "err");
}

TEST_CASE("and_then const rvalue on error short-circuits", "[expected_ref]") {
    const expected<int&, std::string> e(unexpect, "err");
    auto r = std::move(e).and_then([](int& v) -> expected<int, std::string> { return v; });
    REQUIRE(!r.has_value());
}

// --- Monadic: and_then const-rvalue on VALUE state ---
TEST_CASE("and_then const rvalue on value calls F", "[expected_ref]") {
    int                       x = 5;
    const expected<int&, int> e(x);
    auto                      r = std::move(e).and_then([](int& v) -> expected<int, int> { return v * 2; });
    REQUIRE(r.has_value());
    CHECK(*r == 10);
}

// --- Monadic: or_else rvalue/const-rvalue on VALUE state ---
TEST_CASE("or_else rvalue on value short-circuits", "[expected_ref]") {
    int                 x = 42;
    expected<int&, int> e(x);
    auto                r = std::move(e).or_else([](int) -> expected<int&, int> {
        static int dummy = 0;
        return expected<int&, int>(dummy);
    });
    REQUIRE(r.has_value());
    CHECK(*r == 42);
}

TEST_CASE("or_else const rvalue on value short-circuits", "[expected_ref]") {
    int                       x = 42;
    const expected<int&, int> e(x);
    auto                      r = std::move(e).or_else([](int) -> expected<int&, int> {
        static int dummy = 0;
        return expected<int&, int>(dummy);
    });
    REQUIRE(r.has_value());
    CHECK(*r == 42);
}

// --- Monadic: or_else rvalue/const-rvalue on ERROR state ---
TEST_CASE("or_else rvalue on error calls F", "[expected_ref]") {
    expected<int&, int> e(unexpect, 3);
    auto                r = std::move(e).or_else([](int v) -> expected<int&, int> {
        static int result = 0;
        result            = v * 10;
        return expected<int&, int>(result);
    });
    REQUIRE(r.has_value());
    CHECK(*r == 30);
}

TEST_CASE("or_else const rvalue on error calls F", "[expected_ref]") {
    const expected<int&, int> e(unexpect, 3);
    auto                      r = std::move(e).or_else([](int v) -> expected<int&, int> {
        static int result = 0;
        result            = v * 10;
        return expected<int&, int>(result);
    });
    REQUIRE(r.has_value());
    CHECK(*r == 30);
}

// --- Monadic: transform rvalue/const-rvalue on ERROR state ---
TEST_CASE("transform rvalue on error short-circuits", "[expected_ref]") {
    expected<int&, std::string> e(unexpect, "err");
    auto                        r = std::move(e).transform([](int& v) { return v * 2; });
    REQUIRE(!r.has_value());
    CHECK(r.error() == "err");
}

TEST_CASE("transform const rvalue on error short-circuits", "[expected_ref]") {
    const expected<int&, std::string> e(unexpect, "err");
    auto                              r = std::move(e).transform([](int& v) { return v * 2; });
    REQUIRE(!r.has_value());
}

// --- Monadic: transform const-rvalue on VALUE state ---
TEST_CASE("transform const rvalue on value calls F", "[expected_ref]") {
    int                       x = 4;
    const expected<int&, int> e(x);
    auto                      r = std::move(e).transform([](int& v) { return v + 1; });
    REQUIRE(r.has_value());
    CHECK(*r == 5);
}

// --- Monadic: transform_error rvalue/const-rvalue ---
TEST_CASE("transform_error rvalue on value short-circuits", "[expected_ref]") {
    int                 x = 42;
    expected<int&, int> e(x);
    auto                r = std::move(e).transform_error([](int v) { return v * 2; });
    REQUIRE(r.has_value());
    CHECK(*r == 42);
}

TEST_CASE("transform_error const rvalue on value short-circuits", "[expected_ref]") {
    int                       x = 42;
    const expected<int&, int> e(x);
    auto                      r = std::move(e).transform_error([](int v) { return v * 2; });
    REQUIRE(r.has_value());
}

TEST_CASE("transform_error const rvalue on error calls F", "[expected_ref]") {
    const expected<int&, int> e(unexpect, 7);
    auto                      r = std::move(e).transform_error([](int v) { return v + 1; });
    REQUIRE(!r.has_value());
    CHECK(r.error() == 8);
}

TEST_CASE("transform_error const lvalue on error calls F", "[expected_ref]") {
    const expected<int&, int> e(unexpect, 7);
    auto                      r = e.transform_error([](int v) { return v + 1; });
    REQUIRE(!r.has_value());
    CHECK(r.error() == 8);
}

// --- value() throw paths ---
TEST_CASE("value() throws on error state", "[expected_ref]") {
    expected<int&, int> e(unexpect, 42);
    CHECK_THROWS_AS(e.value(), beman::expected::bad_expected_access<int>);
}

// --- Cross-type equality ---
TEST_CASE("equality error vs value", "[expected_ref]") {
    int                 x = 1;
    expected<int&, int> a(x);
    expected<int&, int> b(unexpect, 0);
    CHECK_FALSE(a == b);
}

// ---------------------------------------------------------------------------
// expected<T&, E> with non-trivial E: lvalue monadic overloads
// ---------------------------------------------------------------------------

TEST_CASE("ref<T&,traced>: and_then lvalue value path", "[expected_ref][traced]") {
    int                    x = 5;
    expected<int&, traced> e(x);
    auto                   r = e.and_then([](int& v) -> expected<int, traced> { return v * 2; });
    REQUIRE(r.has_value());
    CHECK(*r == 10);
}

TEST_CASE("ref<T&,traced>: and_then lvalue error path", "[expected_ref][traced]") {
    expected<int&, traced> e(unexpect, 5);
    auto                   r = e.and_then([](int&) -> expected<int, traced> { return 0; });
    REQUIRE(!r.has_value());
}

TEST_CASE("ref<T&,traced>: and_then rvalue error path", "[expected_ref][traced]") {
    expected<int&, traced> e(unexpect, 5);
    auto                   r = std::move(e).and_then([](int&) -> expected<int, traced> { return 0; });
    REQUIRE(!r.has_value());
}

TEST_CASE("ref<T&,traced>: and_then const lvalue error path", "[expected_ref][traced]") {
    const expected<int&, traced> e(unexpect, 5);
    auto                         r = e.and_then([](int&) -> expected<int, traced> { return 0; });
    REQUIRE(!r.has_value());
}

TEST_CASE("ref<T&,traced>: and_then const rvalue error path", "[expected_ref][traced]") {
    const expected<int&, traced> e(unexpect, 5);
    auto                         r = std::move(e).and_then([](int&) -> expected<int, traced> { return 0; });
    REQUIRE(!r.has_value());
}

TEST_CASE("ref<T&,traced>: or_else lvalue value path", "[expected_ref][traced]") {
    int                    x = 5;
    expected<int&, traced> e(x);
    auto                   r = e.or_else([](traced&) -> expected<int&, traced> {
        static int dummy = 0;
        return expected<int&, traced>(dummy);
    });
    REQUIRE(r.has_value());
    CHECK(*r == 5);
}

TEST_CASE("ref<T&,traced>: or_else rvalue value path", "[expected_ref][traced]") {
    int                    x = 5;
    expected<int&, traced> e(x);
    auto                   r = std::move(e).or_else([](traced&&) -> expected<int&, traced> {
        static int dummy = 0;
        return expected<int&, traced>(dummy);
    });
    REQUIRE(r.has_value());
}

TEST_CASE("ref<T&,traced>: or_else const rvalue value path", "[expected_ref][traced]") {
    int                          x = 5;
    const expected<int&, traced> e(x);
    auto                         r = std::move(e).or_else([](const traced&) -> expected<int&, traced> {
        static int dummy = 0;
        return expected<int&, traced>(dummy);
    });
    REQUIRE(r.has_value());
}

TEST_CASE("ref<T&,traced>: transform lvalue value and error paths", "[expected_ref][traced]") {
    int                    x = 4;
    expected<int&, traced> e(x);
    auto                   rv = e.transform([](int& v) { return v + 1; });
    REQUIRE(rv.has_value());
    CHECK(*rv == 5);

    expected<int&, traced> e2(unexpect, 1);
    auto                   re = e2.transform([](int&) { return 0; });
    REQUIRE(!re.has_value());
}

TEST_CASE("ref<T&,traced>: transform rvalue value path", "[expected_ref][traced]") {
    int                    x = 4;
    expected<int&, traced> e(x);
    auto                   r = std::move(e).transform([](int& v) { return v + 1; });
    REQUIRE(r.has_value());
}

TEST_CASE("ref<T&,traced>: transform const rvalue value path", "[expected_ref][traced]") {
    int                          x = 4;
    const expected<int&, traced> e(x);
    auto                         r = std::move(e).transform([](int& v) { return v + 1; });
    REQUIRE(r.has_value());
}

TEST_CASE("ref<T&,traced>: transform to void lvalue", "[expected_ref][traced]") {
    int                    x = 1;
    expected<int&, traced> e(x);
    auto                   r = e.transform([](int&) {});
    REQUIRE(r.has_value());
}

TEST_CASE("ref<T&,traced>: transform_error lvalue value path", "[expected_ref][traced]") {
    int                    x = 5;
    expected<int&, traced> e(x);
    auto                   r = e.transform_error([](traced&) { return traced(0); });
    REQUIRE(r.has_value());
}

TEST_CASE("ref<T&,traced>: transform_error rvalue value path", "[expected_ref][traced]") {
    int                    x = 5;
    expected<int&, traced> e(x);
    auto                   r = std::move(e).transform_error([](traced&&) { return traced(0); });
    REQUIRE(r.has_value());
}

TEST_CASE("ref<T&,traced>: transform_error const lvalue value path", "[expected_ref][traced]") {
    int                          x = 5;
    const expected<int&, traced> e(x);
    auto                         r = e.transform_error([](const traced&) { return traced(0); });
    REQUIRE(r.has_value());
}

TEST_CASE("ref<T&,traced>: transform_error const rvalue value path", "[expected_ref][traced]") {
    int                          x = 5;
    const expected<int&, traced> e(x);
    auto                         r = std::move(e).transform_error([](const traced&) { return traced(0); });
    REQUIRE(r.has_value());
}

TEST_CASE("ref<T&,traced>: transform_error lvalue error path", "[expected_ref][traced]") {
    expected<int&, traced> e(unexpect, 5);
    auto                   r = e.transform_error([](traced& v) { return traced(v.val + 1); });
    REQUIRE(!r.has_value());
    CHECK(r.error().val == 6);
}

TEST_CASE("ref<T&,traced>: copy ctor error state", "[expected_ref][traced]") {
    expected<int&, traced> a(unexpect, 10);
    expected<int&, traced> b(a);
    REQUIRE(!b.has_value());
    CHECK(b.error().val == 10);
}

TEST_CASE("ref<T&,traced>: converting copy ctor error state", "[expected_ref][traced]") {
    expected<int&, narrowed> a(unexpect, narrowed(7));
    expected<int&, widened>  b(a);
    REQUIRE(!b.has_value());
    CHECK(b.error().val == 7);
}

TEST_CASE("ref<T&,traced>: assign unexpected to value state", "[expected_ref][traced]") {
    int                    x = 5;
    expected<int&, traced> e(x);
    e = unexpected<traced>(traced(42));
    REQUIRE(!e.has_value());
    CHECK(e.error().val == 42);
}

TEST_CASE("ref<T&,traced>: move assign error-to-error", "[expected_ref][traced]") {
    expected<int&, traced> a(unexpect, 1);
    expected<int&, traced> b(unexpect, 2);
    b = std::move(a);
    CHECK(b.error().val == 1);
}

TEST_CASE("ref<T&,traced>: swap value and error state", "[expected_ref][traced]") {
    int                    x = 5;
    expected<int&, traced> a(x);
    expected<int&, traced> b(unexpect, 7);
    a.swap(b);
    REQUIRE(!a.has_value());
    CHECK(a.error().val == 7);
    REQUIRE(b.has_value());
    CHECK(*b == 5);
}

TEST_CASE("ref<T&,traced>: equality both errors", "[expected_ref][traced]") {
    expected<int&, traced> a(unexpect, 1);
    expected<int&, traced> b(unexpect, 1);
    CHECK(a == b);
}

TEST_CASE("ref<T&,traced>: equality error vs value", "[expected_ref][traced]") {
    int                    x = 1;
    expected<int&, traced> a(x);
    expected<int&, traced> b(unexpect, 0);
    CHECK_FALSE(a == b);
}
