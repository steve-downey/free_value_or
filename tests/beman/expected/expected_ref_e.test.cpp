// tests/beman/expected/expected_ref_e.test.cpp                        -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/expected/expected.hpp>
#include <beman/expected/expected.hpp>

#include <catch2/catch_test_macros.hpp>

#include "testing/types.hpp"

#include <string>
#include <type_traits>

using namespace beman::expected;

// =============================================================================
// Type-level static assertions
// =============================================================================

// expected<T, E&> is a valid specialization — default constructible (value side)
static_assert(std::is_default_constructible_v<expected<int, int&>>);
static_assert(std::is_constructible_v<expected<int, int&>, std::in_place_t, int>);

// error() returns E& (shallow const — const expected still returns E&, not const E&)
static_assert(std::is_same_v<decltype(std::declval<expected<int, int&>>().error()), int&>);
static_assert(std::is_same_v<decltype(std::declval<const expected<int, int&>>().error()), int&>);

// value() returns T& (non-const) / const T& (const)
static_assert(std::is_same_v<decltype(std::declval<expected<int, int&>&>().value()), int&>);
static_assert(std::is_same_v<decltype(std::declval<const expected<int, int&>&>().value()), const int&>);

// operator-> returns T* / const T*
static_assert(std::is_same_v<decltype(std::declval<expected<int, int&>>().operator->()), int*>);
static_assert(std::is_same_v<decltype(std::declval<const expected<int, int&>>().operator->()), const int*>);

// Copy/move constructible
static_assert(std::is_copy_constructible_v<expected<int, int&>>);
static_assert(std::is_move_constructible_v<expected<int, int&>>);

// Triviality: when T is trivial, copy/move/assign/destroy should be trivial
static_assert(std::is_trivially_copy_constructible_v<expected<int, int&>>);
static_assert(std::is_trivially_move_constructible_v<expected<int, int&>>);
static_assert(std::is_trivially_copy_assignable_v<expected<int, int&>>);
static_assert(std::is_trivially_move_assignable_v<expected<int, int&>>);
static_assert(std::is_trivially_destructible_v<expected<int, int&>>);

// Non-trivial T: still constructible/assignable but not trivially
static_assert(std::is_copy_constructible_v<expected<std::string, int&>>);
static_assert(std::is_move_constructible_v<expected<std::string, int&>>);
static_assert(!std::is_trivially_copy_constructible_v<expected<std::string, int&>>);
static_assert(!std::is_trivially_move_constructible_v<expected<std::string, int&>>);
static_assert(!std::is_trivially_destructible_v<expected<std::string, int&>>);

// Cannot construct from temporary error (rvalue deleted, or any type creating a temp E)
static_assert(!std::is_constructible_v<expected<int, int&>, unexpect_t, int&&>);
// Cross-type temporary: float would create a temp double when binding const double&
static_assert(!std::is_constructible_v<expected<int, const double&>, unexpect_t, float>);
// Lvalue of same type is fine
static_assert(std::is_constructible_v<expected<int, const double&>, unexpect_t, const double&>);

// Converting construction from expected<U, G&>
static_assert(std::is_constructible_v<expected<int, int&>, const expected<long, int&>&>);

// =============================================================================
// Construction — value side (same as primary template)
// =============================================================================

TEST_CASE("expected<T,E&>: default construct has value", "[expected_ref_e]") {
    expected<int, int&> e;
    REQUIRE(e.has_value());
    CHECK(*e == 0);
}

TEST_CASE("expected<T,E&>: construct from value", "[expected_ref_e]") {
    expected<int, int&> e = 42;
    REQUIRE(e.has_value());
    CHECK(*e == 42);
}

TEST_CASE("expected<T,E&>: in_place value construction", "[expected_ref_e]") {
    expected<std::string, int&> e(std::in_place, 3, 'x');
    REQUIRE(e.has_value());
    CHECK(*e == "xxx");
}

// =============================================================================
// Construction — error side (binds reference)
// =============================================================================

TEST_CASE("expected<T,E&>: construct from unexpect lvalue ref", "[expected_ref_e]") {
    int                 err = 7;
    expected<int, int&> e(unexpect, err);
    REQUIRE(!e.has_value());
    CHECK(&e.error() == &err);
    CHECK(e.error() == 7);
}

TEST_CASE("expected<T,E&>: copy construct preserves error pointer", "[expected_ref_e]") {
    int                 err = 42;
    expected<int, int&> a(unexpect, err);
    expected<int, int&> b = a;
    REQUIRE(!b.has_value());
    CHECK(&b.error() == &err);
}

TEST_CASE("expected<T,E&>: move construct preserves error pointer", "[expected_ref_e]") {
    int                 err = 5;
    expected<int, int&> a(unexpect, err);
    expected<int, int&> b = std::move(a);
    REQUIRE(!b.has_value());
    CHECK(&b.error() == &err);
}

// =============================================================================
// Error rebind semantics on assignment
// =============================================================================

TEST_CASE("expected<T,E&>: error rebind via copy assignment", "[expected_ref_e]") {
    int                 err1 = 1, err2 = 2;
    expected<int, int&> a(unexpect, err1);
    expected<int, int&> b(unexpect, err2);
    a = b;
    REQUIRE(!a.has_value());
    CHECK(&a.error() == &err2);
    // err1 unchanged — rebind, not assign-through
    CHECK(err1 == 1);
}

TEST_CASE("expected<T,E&>: rebind does NOT assign through error reference", "[expected_ref_e]") {
    int                 err1 = 10, err2 = 20;
    expected<int, int&> a(unexpect, err1);
    expected<int, int&> b(unexpect, err2);
    a = b;
    CHECK(err1 == 10); // err1 unchanged
    CHECK(a.error() == 20);
}

TEST_CASE("expected<T,E&>: assign value when in error state", "[expected_ref_e]") {
    int                 err = 5;
    expected<int, int&> e(unexpect, err);
    e = 42;
    REQUIRE(e.has_value());
    CHECK(*e == 42);
}

TEST_CASE("expected<T,E&>: assign to error state via expected copy", "[expected_ref_e]") {
    int                 err = 99;
    expected<int, int&> e(42);
    expected<int, int&> src(unexpect, err);
    e = src;
    REQUIRE(!e.has_value());
    CHECK(&e.error() == &err);
}

// Safe alternative to e = unexpected(err): move-assign from a named expected.
// This is the correct idiom when E is a reference — no operator=(unexpected<G>)
// exists for expected<T, E&> because it would bind E& to temporary storage.
TEST_CASE("expected<T,E&>: rebind error via move-assign from temporary expected", "[expected_ref_e]") {
    int                 new_err = 7;
    expected<int, int&> e(42);
    e = expected<int, int&>(unexpect, new_err);
    REQUIRE(!e.has_value());
    CHECK(&e.error() == &new_err);
}

// =============================================================================
// Shallow const on error
// =============================================================================

TEST_CASE("expected<T,E&>: shallow const allows mutation of error referent", "[expected_ref_e]") {
    int                       err = 10;
    const expected<int, int&> e(unexpect, err);
    // error() returns int& (not const int&) — shallow const
    e.error() = 20;
    CHECK(err == 20);
}

// =============================================================================
// Observers
// =============================================================================

TEST_CASE("expected<T,E&>: operator*() and operator->() work normally", "[expected_ref_e]") {
    expected<std::string, int&> e(std::in_place, "hello");
    CHECK(e->size() == 5);
    CHECK(*e == "hello");
}

TEST_CASE("expected<T,E&>: value() returns T& (owned)", "[expected_ref_e]") {
    expected<int, int&> e(42);
    static_assert(std::is_same_v<decltype(e.value()), int&>);
    e.value() = 99;
    CHECK(*e == 99);
}

TEST_CASE("expected<T,E&>: value() throws on error", "[expected_ref_e]") {
    int                 err = 5;
    expected<int, int&> e(unexpect, err);
    REQUIRE_THROWS_AS(e.value(), beman::expected::bad_expected_access<int>);
}

TEST_CASE("expected<T,E&>: error() returns E&", "[expected_ref_e]") {
    int                 err = 7;
    expected<int, int&> e(unexpect, err);
    static_assert(std::is_same_v<decltype(e.error()), int&>);
    CHECK(&e.error() == &err);
}

TEST_CASE("expected<T,E&>: value_or works normally for value side", "[expected_ref_e]") {
    int                 err = 0;
    expected<int, int&> a(42);
    expected<int, int&> b(unexpect, err);
    CHECK(a.value_or(0) == 42);
    CHECK(b.value_or(99) == 99);
}

TEST_CASE("expected<T,E&>: error_or returns E by value", "[expected_ref_e]") {
    int                 err = 7;
    expected<int, int&> a(unexpect, err);
    expected<int, int&> b(42);
    CHECK(a.error_or(0) == 7);
    CHECK(b.error_or(0) == 0);
}

// =============================================================================
// Swap
// =============================================================================

TEST_CASE("expected<T,E&>: swap value-value", "[expected_ref_e]") {
    expected<int, int&> a(1), b(2);
    a.swap(b);
    CHECK(*a == 2);
    CHECK(*b == 1);
}

TEST_CASE("expected<T,E&>: swap value-error", "[expected_ref_e]") {
    int                 err = 99;
    expected<int, int&> a(1), b(unexpect, err);
    a.swap(b);
    REQUIRE(!a.has_value());
    REQUIRE(b.has_value());
    CHECK(&a.error() == &err);
    CHECK(*b == 1);
}

TEST_CASE("expected<T,E&>: swap error-value", "[expected_ref_e]") {
    int                 err = 99;
    expected<int, int&> a(unexpect, err), b(1);
    a.swap(b);
    REQUIRE(a.has_value());
    REQUIRE(!b.has_value());
    CHECK(*a == 1);
    CHECK(&b.error() == &err);
}

TEST_CASE("expected<T,E&>: swap error-error rebinds pointers", "[expected_ref_e]") {
    int                 e1 = 1, e2 = 2;
    expected<int, int&> a(unexpect, e1), b(unexpect, e2);
    a.swap(b);
    CHECK(&a.error() == &e2);
    CHECK(&b.error() == &e1);
}

// =============================================================================
// Equality
// =============================================================================

TEST_CASE("expected<T,E&>: equality of two value-holding instances", "[expected_ref_e]") {
    expected<int, int&> a(42), b(42);
    CHECK(a == b);
}

TEST_CASE("expected<T,E&>: inequality when values differ", "[expected_ref_e]") {
    expected<int, int&> a(1), b(2);
    CHECK(!(a == b));
}

TEST_CASE("expected<T,E&>: equality with value type", "[expected_ref_e]") {
    expected<int, int&> e(42);
    CHECK(e == 42);
    CHECK(!(e == 99));
}

TEST_CASE("expected<T,E&>: equality with unexpected (compares error values)", "[expected_ref_e]") {
    int                 err = 7;
    expected<int, int&> e(unexpect, err);
    // Compares by value, not by pointer identity
    CHECK(e == unexpected(7));
    CHECK(!(e == unexpected(8)));
}

TEST_CASE("expected<T,E&>: value vs error always unequal", "[expected_ref_e]") {
    int                 err = 0;
    expected<int, int&> a(0), b(unexpect, err);
    CHECK(!(a == b));
}

// =============================================================================
// Monadic operations
// =============================================================================

TEST_CASE("expected<T,E&>: and_then works on value side", "[expected_ref_e]") {
    expected<int, int&> e(5);
    auto                r = e.and_then([](int v) -> expected<int, int&> { return v * 2; });
    REQUIRE(r.has_value());
    CHECK(*r == 10);
}

TEST_CASE("expected<T,E&>: and_then propagates error ref", "[expected_ref_e]") {
    int                 err = 3;
    expected<int, int&> e(unexpect, err);
    auto                r = e.and_then([](int v) -> expected<int, int&> { return v * 2; });
    REQUIRE(!r.has_value());
    CHECK(&r.error() == &err);
}

TEST_CASE("expected<T,E&>: or_else receives E& and can inspect error", "[expected_ref_e]") {
    int                 err = 3;
    expected<int, int&> e(unexpect, err);
    auto                r = e.or_else([](int& v) -> expected<int, int&> { return v * 10; });
    REQUIRE(r.has_value());
    CHECK(*r == 30);
}

TEST_CASE("expected<T,E&>: or_else propagates value", "[expected_ref_e]") {
    expected<int, int&> e(42);
    auto                r = e.or_else([](int& v) -> expected<int, int&> { return v + 1; });
    REQUIRE(r.has_value());
    CHECK(*r == 42);
}

TEST_CASE("expected<T,E&>: transform transforms value", "[expected_ref_e]") {
    expected<int, int&> e(5);
    auto                r = e.transform([](int v) { return v * 2; });
    REQUIRE(r.has_value());
    CHECK(*r == 10);
}

TEST_CASE("expected<T,E&>: transform propagates error ref", "[expected_ref_e]") {
    int                 err = 5;
    expected<int, int&> e(unexpect, err);
    auto                r = e.transform([](int v) { return v * 2; });
    REQUIRE(!r.has_value());
    CHECK(&r.error() == &err);
}

TEST_CASE("expected<T,E&>: transform_error transforms E&", "[expected_ref_e]") {
    int                 err = 5;
    expected<int, int&> e(unexpect, err);
    auto                r = e.transform_error([](int& v) -> std::string { return std::to_string(v); });
    REQUIRE(!r.has_value());
    CHECK(r.error() == "5");
}

TEST_CASE("expected<T,E&>: transform_error propagates value", "[expected_ref_e]") {
    expected<int, int&> e(42);
    auto                r = e.transform_error([](int& v) -> std::string { return std::to_string(v); });
    REQUIRE(r.has_value());
    CHECK(*r == 42);
}

// =============================================================================
// Converting construction from expected<U, G&>
// =============================================================================

TEST_CASE("expected<T,E&>: converting copy construct from expected<U,G&> with value", "[expected_ref_e]") {
    expected<long, int&> src(42L);
    expected<int, int&>  e(src);
    REQUIRE(e.has_value());
    CHECK(*e == 42);
}

TEST_CASE("expected<T,E&>: converting copy construct from expected<U,G&> with error", "[expected_ref_e]") {
    int                  err = 7;
    expected<long, int&> src(unexpect, err);
    expected<int, int&>  e(src);
    REQUIRE(!e.has_value());
    CHECK(&e.error() == &err);
}

TEST_CASE("expected<T,E&>: converting move construct from expected<U,G&> with value", "[expected_ref_e]") {
    expected<long, int&> src(99L);
    expected<int, int&>  e(std::move(src));
    REQUIRE(e.has_value());
    CHECK(*e == 99);
}

TEST_CASE("expected<T,E&>: converting move construct from expected<U,G&> preserves error pointer",
          "[expected_ref_e]") {
    int                  err = 5;
    expected<long, int&> src(unexpect, err);
    expected<int, int&>  e(std::move(src));
    REQUIRE(!e.has_value());
    CHECK(&e.error() == &err);
}

// =============================================================================
// Dangling prevention — verify lvalue binding works
// =============================================================================

TEST_CASE("expected<T,E&>: lvalue error reference compiles and is addressable", "[expected_ref_e]") {
    int                 err = 1;
    expected<int, int&> e(unexpect, err);
    CHECK(&e.error() == &err);
}

// =============================================================================
// Additional coverage tests: T,E& rvalue/const-rvalue monadic and traced paths
// =============================================================================
using namespace beman::expected::testing;

// --- Monadic: and_then rvalue/const-rvalue on ERROR state ---
TEST_CASE("and_then rvalue on error short-circuits", "[expected_ref_e]") {
    int                 err = 5;
    expected<int, int&> e(unexpect, err);
    auto                r = std::move(e).and_then([](int) -> expected<int, int&> {
        static int dummy = 0;
        return expected<int, int&>(unexpect, dummy);
    });
    REQUIRE(!r.has_value());
    CHECK(r.error() == 5);
}

TEST_CASE("and_then const rvalue on error short-circuits", "[expected_ref_e]") {
    int                       err = 5;
    const expected<int, int&> e(unexpect, err);
    auto                      r = std::move(e).and_then([](int) -> expected<int, int&> {
        static int dummy = 0;
        return expected<int, int&>(unexpect, dummy);
    });
    REQUIRE(!r.has_value());
}

// --- Monadic: or_else rvalue/const-rvalue on VALUE state ---
TEST_CASE("or_else rvalue on value short-circuits", "[expected_ref_e]") {
    expected<int, int&> e(42);
    auto                r = std::move(e).or_else([](int&) -> expected<int, int&> {
        static int dummy = 0;
        return expected<int, int&>(unexpect, dummy);
    });
    REQUIRE(r.has_value());
    CHECK(*r == 42);
}

TEST_CASE("or_else const rvalue on value short-circuits", "[expected_ref_e]") {
    const expected<int, int&> e(42);
    auto                      r = std::move(e).or_else([](int&) -> expected<int, int&> {
        static int dummy = 0;
        return expected<int, int&>(unexpect, dummy);
    });
    REQUIRE(r.has_value());
}

// --- Monadic: transform rvalue/const-rvalue on ERROR state ---
TEST_CASE("transform rvalue on error short-circuits", "[expected_ref_e]") {
    int                 err = 3;
    expected<int, int&> e(unexpect, err);
    auto                r = std::move(e).transform([](int v) { return v * 2; });
    REQUIRE(!r.has_value());
}

TEST_CASE("transform const rvalue on error short-circuits", "[expected_ref_e]") {
    int                       err = 3;
    const expected<int, int&> e(unexpect, err);
    auto                      r = std::move(e).transform([](int v) { return v * 2; });
    REQUIRE(!r.has_value());
}

// --- Monadic: transform_error rvalue/const-rvalue ---
TEST_CASE("transform_error rvalue on value short-circuits", "[expected_ref_e]") {
    expected<int, int&> e(42);
    auto                r = std::move(e).transform_error([](int& v) { return v * 2; });
    REQUIRE(r.has_value());
    CHECK(*r == 42);
}

TEST_CASE("transform_error const rvalue on value short-circuits", "[expected_ref_e]") {
    const expected<int, int&> e(42);
    auto                      r = std::move(e).transform_error([](int& v) { return v * 2; });
    REQUIRE(r.has_value());
}

TEST_CASE("transform_error rvalue on error calls F", "[expected_ref_e]") {
    int                 err = 5;
    expected<int, int&> e(unexpect, err);
    auto                r = std::move(e).transform_error([](int& v) { return v + 1; });
    REQUIRE(!r.has_value());
    CHECK(r.error() == 6);
}

TEST_CASE("transform_error const rvalue on error calls F", "[expected_ref_e]") {
    int                       err = 5;
    const expected<int, int&> e(unexpect, err);
    auto                      r = std::move(e).transform_error([](int& v) { return v + 1; });
    REQUIRE(!r.has_value());
    CHECK(r.error() == 6);
}

// --- value assignment on value-state ---
TEST_CASE("value assignment on value state", "[expected_ref_e]") {
    expected<int, int&> e(10);
    e = 20;
    REQUIRE(e.has_value());
    CHECK(*e == 20);
}

// --- Cross-type equality ---
TEST_CASE("equality error vs value", "[expected_ref_e]") {
    int                 err = 0;
    expected<int, int&> a(1);
    expected<int, int&> b(unexpect, err);
    CHECK_FALSE(a == b);
}

// ---------------------------------------------------------------------------
// expected<T, E&>: lvalue monadic paths with non-trivial T
// ---------------------------------------------------------------------------

TEST_CASE("ref<traced,E&>: and_then lvalue error path", "[expected_ref_e][traced]") {
    int                    err = 5;
    expected<traced, int&> e(unexpect, err);
    auto                   r = e.and_then([](traced&) -> expected<traced, int&> {
        static int dummy = 0;
        return expected<traced, int&>(unexpect, dummy);
    });
    REQUIRE(!r.has_value());
}

TEST_CASE("ref<traced,E&>: or_else lvalue value path", "[expected_ref_e][traced]") {
    expected<traced, int&> e(std::in_place, 5);
    auto                   r = e.or_else([](int&) -> expected<traced, int&> {
        static int dummy = 0;
        return expected<traced, int&>(unexpect, dummy);
    });
    REQUIRE(r.has_value());
    CHECK(r->val == 5);
}

TEST_CASE("ref<traced,E&>: or_else rvalue value path", "[expected_ref_e][traced]") {
    expected<traced, int&> e(std::in_place, 5);
    auto                   r = std::move(e).or_else([](int&) -> expected<traced, int&> {
        static int dummy = 0;
        return expected<traced, int&>(unexpect, dummy);
    });
    REQUIRE(r.has_value());
}

TEST_CASE("ref<traced,E&>: or_else const rvalue value path", "[expected_ref_e][traced]") {
    const expected<traced, int&> e(std::in_place, 5);
    auto                         r = std::move(e).or_else([](int&) -> expected<traced, int&> {
        static int dummy = 0;
        return expected<traced, int&>(unexpect, dummy);
    });
    REQUIRE(r.has_value());
}

TEST_CASE("ref<traced,E&>: transform lvalue value path", "[expected_ref_e][traced]") {
    expected<traced, int&> e(std::in_place, 3);
    auto                   r = e.transform([](traced& v) { return traced(v.val + 1); });
    REQUIRE(r.has_value());
}

TEST_CASE("ref<traced,E&>: transform rvalue value path", "[expected_ref_e][traced]") {
    expected<traced, int&> e(std::in_place, 3);
    auto                   r = std::move(e).transform([](traced&& v) { return traced(v.val + 1); });
    REQUIRE(r.has_value());
}

TEST_CASE("ref<traced,E&>: transform const rvalue value path", "[expected_ref_e][traced]") {
    const expected<traced, int&> e(std::in_place, 3);
    auto                         r = std::move(e).transform([](const traced& v) { return traced(v.val + 1); });
    REQUIRE(r.has_value());
}

TEST_CASE("ref<traced,E&>: transform_error lvalue value path", "[expected_ref_e][traced]") {
    expected<traced, int&> e(std::in_place, 5);
    auto                   r = e.transform_error([](int& v) { return v * 2; });
    REQUIRE(r.has_value());
}

TEST_CASE("ref<traced,E&>: transform_error rvalue value path", "[expected_ref_e][traced]") {
    expected<traced, int&> e(std::in_place, 5);
    auto                   r = std::move(e).transform_error([](int& v) { return v * 2; });
    REQUIRE(r.has_value());
}

TEST_CASE("ref<traced,E&>: transform_error const rvalue value path", "[expected_ref_e][traced]") {
    const expected<traced, int&> e(std::in_place, 5);
    auto                         r = std::move(e).transform_error([](int& v) { return v * 2; });
    REQUIRE(r.has_value());
}

TEST_CASE("ref<traced,E&>: equality error vs value", "[expected_ref_e][traced]") {
    int                    err = 0;
    expected<traced, int&> a(std::in_place, 1);
    expected<traced, int&> b(unexpect, err);
    CHECK_FALSE(a == b);
}
