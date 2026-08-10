// tests/beman/expected/expected_ref_both.test.cpp                      -*-C++-*-
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
// nothing. Type identity is checked by comparing type_name, which compares by
// std::is_same_v — exactly as strict as the original — with the compiler's
// spellings used only to explain a failure.
// =============================================================================

TEST_CASE("expected<T&,E&>: special member availability", "[expected_ref_both]") {
    // No default constructor — T& cannot be default-initialized
    CHECK_FALSE(std::is_default_constructible_v<expected<int&, int&>>);

    // Constructible from lvalue (value side)
    CHECK(std::is_constructible_v<expected<int&, int&>, int&>);

    // Copy/move constructible
    CHECK(std::is_copy_constructible_v<expected<int&, int&>>);
    CHECK(std::is_move_constructible_v<expected<int&, int&>>);
}

TEST_CASE("expected<T&,E&>: fully trivial — both sides are pointers", "[expected_ref_both]") {
    // Trivially copyable/movable/destructible
    CHECK(std::is_trivially_copy_constructible_v<expected<int&, int&>>);
    CHECK(std::is_trivially_move_constructible_v<expected<int&, int&>>);
    CHECK(std::is_trivially_copy_assignable_v<expected<int&, int&>>);
    CHECK(std::is_trivially_move_assignable_v<expected<int&, int&>>);
    CHECK(std::is_trivially_destructible_v<expected<int&, int&>>);
}

TEST_CASE("expected<T&,const E&>: assignable even though const E& is not", "[expected_ref_both]") {
    // Finding 1: copy/move assignment must be available for const-reference E, where E
    // itself is not assignable (is_copy_assignable_v<const int&> is false) but the
    // stored unexpected<E&> rebinds via pointer assignment.
    CHECK(std::is_copy_assignable_v<expected<int&, const int&>>);
    CHECK(std::is_move_assignable_v<expected<int&, const int&>>);
}

TEST_CASE("expected<T&,E&>: observer return types are shallow-const", "[expected_ref_both]") {
    using expected_t = expected<int&, int&>;

    // operator-> returns T* (shallow const)
    CHECK(type_name<decltype(std::declval<expected_t>().operator->())>() == type_name<int*>());
    CHECK(type_name<decltype(std::declval<const expected_t>().operator->())>() == type_name<int*>());

    // operator* returns T& (shallow const)
    CHECK(type_name<decltype(*std::declval<expected_t>())>() == type_name<int&>());
    CHECK(type_name<decltype(*std::declval<const expected_t>())>() == type_name<int&>());

    // value() returns T& (shallow const)
    CHECK(type_name<decltype(std::declval<expected_t>().value())>() == type_name<int&>());
    CHECK(type_name<decltype(std::declval<const expected_t>().value())>() == type_name<int&>());

    // error() returns E& (shallow const)
    CHECK(type_name<decltype(std::declval<expected_t>().error())>() == type_name<int&>());
    CHECK(type_name<decltype(std::declval<const expected_t>().error())>() == type_name<int&>());
}

TEST_CASE("expected<T&,E&>: temporaries cannot be bound", "[expected_ref_both]") {
    // Cannot construct from temporary value (T& rvalue deleted)
    CHECK_FALSE(std::is_constructible_v<expected<int&, int&>, int&&>);

    // Cannot construct from temporary error (rvalue or any type creating a temp E)
    CHECK_FALSE(std::is_constructible_v<expected<int&, int&>, unexpect_t, int&&>);
    // Cross-type temporary: float would create a temp double when binding const double&
    CHECK_FALSE(std::is_constructible_v<expected<int&, const double&>, unexpect_t, float>);
    // Lvalue of same type is fine
    CHECK(std::is_constructible_v<expected<int&, const double&>, unexpect_t, const double&>);
}

TEST_CASE("expected<T&,E&>: converting construction from expected<U&,G&> is available", "[expected_ref_both]") {
    CHECK(std::is_constructible_v<expected<int&, int&>, const expected<int&, int&>&>);
}

// =============================================================================
// Construction — value side
// =============================================================================

TEST_CASE("expected<T&,E&>: construct from lvalue reference (value)", "[expected_ref_both]") {
    int                  x = 42;
    expected<int&, int&> e(x);
    REQUIRE(e.has_value());
    CHECK(&*e == &x);
    CHECK(*e == 42);
}

TEST_CASE("expected<T&,E&>: operator-> returns T*", "[expected_ref_both]") {
    std::string                  s = "hello";
    expected<std::string&, int&> e(s);
    REQUIRE(e.has_value());
    CHECK(e->size() == 5);
}

// =============================================================================
// Construction — error side
// =============================================================================

TEST_CASE("expected<T&,E&>: construct from error lvalue reference", "[expected_ref_both]") {
    int                  err = 7;
    expected<int&, int&> e(unexpect, err);
    REQUIRE(!e.has_value());
    CHECK(&e.error() == &err);
    CHECK(e.error() == 7);
}

// =============================================================================
// Copy and move construction
// =============================================================================

TEST_CASE("expected<T&,E&>: copy construct preserves value pointer", "[expected_ref_both]") {
    int                  x = 42;
    expected<int&, int&> a(x);
    expected<int&, int&> b = a;
    REQUIRE(b.has_value());
    CHECK(&*b == &x);
}

TEST_CASE("expected<T&,E&>: copy construct preserves error pointer", "[expected_ref_both]") {
    int                  err = 5;
    expected<int&, int&> a(unexpect, err);
    expected<int&, int&> b = a;
    REQUIRE(!b.has_value());
    CHECK(&b.error() == &err);
}

TEST_CASE("expected<T&,E&>: move construct preserves value pointer", "[expected_ref_both]") {
    int                  x = 42;
    expected<int&, int&> a(x);
    expected<int&, int&> b = std::move(a);
    REQUIRE(b.has_value());
    CHECK(&*b == &x);
}

TEST_CASE("expected<T&,E&>: move construct preserves error pointer", "[expected_ref_both]") {
    int                  err = 5;
    expected<int&, int&> a(unexpect, err);
    expected<int&, int&> b = std::move(a);
    REQUIRE(!b.has_value());
    CHECK(&b.error() == &err);
}

// =============================================================================
// Value rebind semantics
// =============================================================================

TEST_CASE("expected<T&,E&>: value rebind via copy assignment", "[expected_ref_both]") {
    int                  x1 = 1, x2 = 2;
    expected<int&, int&> a(x1);
    expected<int&, int&> b(x2);
    a = b;
    REQUIRE(a.has_value());
    CHECK(&*a == &x2);
    // x1 unchanged — rebind, not assign-through
    CHECK(x1 == 1);
}

TEST_CASE("expected<T&,E&>: rebind does NOT assign through value reference", "[expected_ref_both]") {
    int                  x1 = 10, x2 = 20;
    expected<int&, int&> a(x1);
    expected<int&, int&> b(x2);
    a = b;
    CHECK(x1 == 10); // x1 unchanged
    CHECK(*a == 20);
}

TEST_CASE("expected<T&,E&>: value rebind operator=(U&&)", "[expected_ref_both]") {
    int                  x1 = 1, x2 = 99;
    expected<int&, int&> e(x1);
    e = x2;
    REQUIRE(e.has_value());
    CHECK(&*e == &x2);
    CHECK(x1 == 1); // x1 unchanged
}

TEST_CASE("expected<T&,E&>: transition from error to value via rebind", "[expected_ref_both]") {
    int                  x = 42, err = 5;
    expected<int&, int&> e(unexpect, err);
    REQUIRE(!e.has_value());
    e = x;
    REQUIRE(e.has_value());
    CHECK(&*e == &x);
}

// =============================================================================
// Error rebind semantics
// =============================================================================

TEST_CASE("expected<T&,E&>: error rebind via copy assignment", "[expected_ref_both]") {
    int                  e1 = 1, e2 = 2;
    expected<int&, int&> a(unexpect, e1);
    expected<int&, int&> b(unexpect, e2);
    a = b;
    REQUIRE(!a.has_value());
    CHECK(&a.error() == &e2);
    // e1 unchanged — rebind, not assign-through
    CHECK(e1 == 1);
}

TEST_CASE("expected<T&,E&>: transition from value to error via copy assignment", "[expected_ref_both]") {
    int                  x = 42, err = 5;
    expected<int&, int&> a(x);
    expected<int&, int&> b(unexpect, err);
    a = b;
    REQUIRE(!a.has_value());
    CHECK(&a.error() == &err);
}

TEST_CASE("expected<T&,const E&>: copy assignment rebinds error, does not assign through", "[expected_ref_both]") {
    int                        e1 = 1, e2 = 2;
    expected<int&, const int&> a(unexpect, e1);
    expected<int&, const int&> b(unexpect, e2);
    a = b;
    REQUIRE(!a.has_value());
    CHECK(&a.error() == &e2);
    CHECK(e1 == 1);
}

TEST_CASE("expected<T&,const E&>: move assignment rebinds error, does not assign through", "[expected_ref_both]") {
    int                        e1 = 1, e2 = 2;
    expected<int&, const int&> a(unexpect, e1);
    expected<int&, const int&> b(unexpect, e2);
    a = std::move(b);
    REQUIRE(!a.has_value());
    CHECK(&a.error() == &e2);
    CHECK(e1 == 1);
}

// Safe alternative to e = unexpected(err): move-assign from a named expected.
// No operator=(unexpected<G>) exists for expected<T&, E&> — it would bind E&
// to temporary storage creating a dangling reference.
TEST_CASE("expected<T&,E&>: rebind error via move-assign from temporary expected", "[expected_ref_both]") {
    int                  x = 1, new_err = 7;
    expected<int&, int&> e(x);
    e = expected<int&, int&>(unexpect, new_err);
    REQUIRE(!e.has_value());
    CHECK(&e.error() == &new_err);
}

// =============================================================================
// emplace
// =============================================================================

TEST_CASE("expected<T&,E&>: emplace rebinds value reference", "[expected_ref_both]") {
    int                  x1 = 1, x2 = 99;
    expected<int&, int&> e(x1);
    int&                 r = e.emplace(x2);
    REQUIRE(e.has_value());
    CHECK(&r == &x2);
    CHECK(&*e == &x2);
    CHECK(x1 == 1); // x1 unchanged
}

TEST_CASE("expected<T&,E&>: emplace from error state", "[expected_ref_both]") {
    int                  x = 42, err = 5;
    expected<int&, int&> e(unexpect, err);
    e.emplace(x);
    REQUIRE(e.has_value());
    CHECK(&*e == &x);
}

// =============================================================================
// Shallow const on both sides
// =============================================================================

TEST_CASE("expected<T&,E&>: shallow const allows mutation of value referent", "[expected_ref_both]") {
    int                        x = 10;
    const expected<int&, int&> e(x);
    *e = 20;
    CHECK(x == 20);
}

TEST_CASE("expected<T&,E&>: shallow const allows mutation of error referent", "[expected_ref_both]") {
    int                        err = 10;
    const expected<int&, int&> e(unexpect, err);
    e.error() = 20;
    CHECK(err == 20);
}

// =============================================================================
// Observers
// =============================================================================

TEST_CASE("expected<T&,E&>: operator*() returns T& (mutation visible)", "[expected_ref_both]") {
    int                  x = 42;
    expected<int&, int&> e(x);
    *e = 99;
    CHECK(x == 99);
}

TEST_CASE("expected<T&,E&>: value() returns T& (throws on error)", "[expected_ref_both]") {
    int                  x = 42;
    expected<int&, int&> e(x);
    CHECK(type_name<decltype(e.value())>() == type_name<int&>());
    CHECK(&e.value() == &x);
}

TEST_CASE("expected<T&,E&>: value() throws bad_expected_access on error", "[expected_ref_both]") {
    int                  err = 5;
    expected<int&, int&> e(unexpect, err);
    REQUIRE_THROWS_AS(e.value(), beman::expected::bad_expected_access<int>);
}

TEST_CASE("expected<T&,E&>: error() returns E& (mutation visible)", "[expected_ref_both]") {
    int                  err = 7;
    expected<int&, int&> e(unexpect, err);
    CHECK(type_name<decltype(e.error())>() == type_name<int&>());
    e.error() = 99;
    CHECK(err == 99);
}

TEST_CASE("expected<T&,E&>: value_or returns T by value", "[expected_ref_both]") {
    int                  x = 42, err = 0;
    expected<int&, int&> a(x);
    expected<int&, int&> b(unexpect, err);
    CHECK(a.value_or(0) == 42);
    CHECK(b.value_or(99) == 99);
}

TEST_CASE("expected<T&,E&>: error_or returns E by value", "[expected_ref_both]") {
    int                  err = 7, x = 0;
    expected<int&, int&> a(unexpect, err);
    expected<int&, int&> b(x);
    CHECK(a.error_or(0) == 7);
    CHECK(b.error_or(99) == 99);
}

// =============================================================================
// Swap
// =============================================================================

TEST_CASE("expected<T&,E&>: swap value-value rebinds pointers", "[expected_ref_both]") {
    int                  x1 = 1, x2 = 2;
    expected<int&, int&> a(x1), b(x2);
    a.swap(b);
    CHECK(&*a == &x2);
    CHECK(&*b == &x1);
    CHECK(x1 == 1); // values unchanged — just rebind
    CHECK(x2 == 2);
}

TEST_CASE("expected<T&,E&>: swap error-error rebinds pointers", "[expected_ref_both]") {
    int                  e1 = 1, e2 = 2;
    expected<int&, int&> a(unexpect, e1), b(unexpect, e2);
    a.swap(b);
    CHECK(&a.error() == &e2);
    CHECK(&b.error() == &e1);
}

TEST_CASE("expected<T&,E&>: swap value-error", "[expected_ref_both]") {
    int                  x = 42, err = 5;
    expected<int&, int&> a(x), b(unexpect, err);
    a.swap(b);
    REQUIRE(!a.has_value());
    REQUIRE(b.has_value());
    CHECK(&a.error() == &err);
    CHECK(&*b == &x);
}

TEST_CASE("expected<T&,E&>: swap error-value", "[expected_ref_both]") {
    int                  x = 42, err = 5;
    expected<int&, int&> a(unexpect, err), b(x);
    a.swap(b);
    REQUIRE(a.has_value());
    REQUIRE(!b.has_value());
    CHECK(&*a == &x);
    CHECK(&b.error() == &err);
}

// =============================================================================
// Equality
// =============================================================================

TEST_CASE("expected<T&,E&>: equality of two value-holding instances", "[expected_ref_both]") {
    int                  x1 = 42, x2 = 42;
    expected<int&, int&> a(x1), b(x2);
    CHECK(a == b);
}

TEST_CASE("expected<T&,E&>: inequality when values differ", "[expected_ref_both]") {
    int                  x1 = 1, x2 = 2;
    expected<int&, int&> a(x1), b(x2);
    CHECK(!(a == b));
}

TEST_CASE("expected<T&,E&>: equality with value type", "[expected_ref_both]") {
    int                  x = 42;
    expected<int&, int&> e(x);
    CHECK(e == 42);
    CHECK(!(e == 99));
}

TEST_CASE("expected<T&,E&>: equality with unexpected (compares error values)", "[expected_ref_both]") {
    int                  err = 7;
    expected<int&, int&> e(unexpect, err);
    CHECK(e == unexpected(7));
    CHECK(!(e == unexpected(8)));
}

TEST_CASE("expected<T&,E&>: value vs error always unequal", "[expected_ref_both]") {
    int                  x = 0, err = 0;
    expected<int&, int&> a(x), b(unexpect, err);
    CHECK(!(a == b));
}

// =============================================================================
// Monadic operations
// =============================================================================

TEST_CASE("expected<T&,E&>: and_then works on value side", "[expected_ref_both]") {
    int                  x = 5;
    expected<int&, int&> e(x);
    auto                 r = e.and_then([](int& v) -> expected<int, int&> { return v * 2; });
    REQUIRE(r.has_value());
    CHECK(*r == 10);
}

TEST_CASE("expected<T&,E&>: and_then propagates error ref", "[expected_ref_both]") {
    int                  err = 3;
    expected<int&, int&> e(unexpect, err);
    auto                 r = e.and_then([](int& v) -> expected<int&, int&> { return v; });
    REQUIRE(!r.has_value());
    CHECK(&r.error() == &err);
}

TEST_CASE("expected<T&,E&>: or_else receives E& and can inspect error", "[expected_ref_both]") {
    int                  err = 3;
    expected<int&, int&> e(unexpect, err);
    int                  result_val = 0;
    auto                 r          = e.or_else([&result_val](int& v) -> expected<int&, int&> {
        result_val = v * 10;
        return result_val;
    });
    REQUIRE(r.has_value());
    CHECK(*r == 30);
}

TEST_CASE("expected<T&,E&>: or_else propagates value ref", "[expected_ref_both]") {
    int                  x = 42;
    expected<int&, int&> e(x);
    int                  other_err = 0;
    auto                 r =
        e.or_else([&other_err](int&) -> expected<int&, int&> { return expected<int&, int&>(unexpect, other_err); });
    REQUIRE(r.has_value());
    CHECK(&*r == &x);
}

TEST_CASE("expected<T&,E&>: transform transforms value", "[expected_ref_both]") {
    int                  x = 5;
    expected<int&, int&> e(x);
    auto                 r = e.transform([](int& v) { return v * 2; });
    REQUIRE(r.has_value());
    CHECK(*r == 10);
}

TEST_CASE("expected<T&,E&>: transform propagates error ref", "[expected_ref_both]") {
    int                  err = 5;
    expected<int&, int&> e(unexpect, err);
    auto                 r = e.transform([](int& v) { return v * 2; });
    REQUIRE(!r.has_value());
    CHECK(&r.error() == &err);
}

TEST_CASE("expected<T&,E&>: transform_error transforms E&", "[expected_ref_both]") {
    int                  err = 5;
    expected<int&, int&> e(unexpect, err);
    auto                 r = e.transform_error([](int& v) -> std::string { return std::to_string(v); });
    REQUIRE(!r.has_value());
    CHECK(r.error() == "5");
}

TEST_CASE("expected<T&,E&>: transform_error propagates value ref", "[expected_ref_both]") {
    int                  x = 42;
    expected<int&, int&> e(x);
    auto                 r = e.transform_error([](int& v) -> std::string { return std::to_string(v); });
    REQUIRE(r.has_value());
    CHECK(&*r == &x);
}

// =============================================================================
// Converting construction from expected<U&, G&>
// =============================================================================

TEST_CASE("expected<T&,E&>: converting copy construct with value", "[expected_ref_both]") {
    int                  x = 42;
    expected<int&, int&> src(x);
    expected<int&, int&> e(src);
    REQUIRE(e.has_value());
    CHECK(&*e == &x);
}

TEST_CASE("expected<T&,E&>: converting copy construct with error", "[expected_ref_both]") {
    int                  err = 7;
    expected<int&, int&> src(unexpect, err);
    expected<int&, int&> e(src);
    REQUIRE(!e.has_value());
    CHECK(&e.error() == &err);
}

// =============================================================================
// Dangling prevention — verify lvalue bindings work
// =============================================================================

TEST_CASE("expected<T&,E&>: lvalue value reference compiles and is addressable", "[expected_ref_both]") {
    int                  x = 1;
    expected<int&, int&> e(x);
    CHECK(&*e == &x);
}

TEST_CASE("expected<T&,E&>: lvalue error reference compiles and is addressable", "[expected_ref_both]") {
    int                  err = 1;
    expected<int&, int&> e(unexpect, err);
    CHECK(&e.error() == &err);
}

// =============================================================================
// Additional coverage tests: T&,E& rvalue/const-rvalue monadic paths
// =============================================================================
using namespace beman::expected::testing;

// --- Monadic: and_then rvalue/const-rvalue on ERROR state ---
TEST_CASE("and_then rvalue on error short-circuits", "[expected_ref_both]") {
    int                  err = 5;
    expected<int&, int&> e(unexpect, err);
    auto                 r = std::move(e).and_then([](int&) -> expected<int, int&> {
        static int dummy = 0;
        return expected<int, int&>(unexpect, dummy);
    });
    REQUIRE(!r.has_value());
}

TEST_CASE("and_then const rvalue on error short-circuits", "[expected_ref_both]") {
    int                        err = 5;
    const expected<int&, int&> e(unexpect, err);
    auto                       r = std::move(e).and_then([](int&) -> expected<int, int&> {
        static int dummy = 0;
        return expected<int, int&>(unexpect, dummy);
    });
    REQUIRE(!r.has_value());
}

// --- Monadic: or_else rvalue/const-rvalue on VALUE state ---
TEST_CASE("or_else rvalue on value short-circuits", "[expected_ref_both]") {
    int                  x = 42;
    expected<int&, int&> e(x);
    auto                 r = std::move(e).or_else([](int&) -> expected<int&, int&> {
        static int dummy = 0;
        return expected<int&, int&>(dummy);
    });
    REQUIRE(r.has_value());
    CHECK(*r == 42);
}

TEST_CASE("or_else const rvalue on value short-circuits", "[expected_ref_both]") {
    int                        x = 42;
    const expected<int&, int&> e(x);
    auto                       r = std::move(e).or_else([](int&) -> expected<int&, int&> {
        static int dummy = 0;
        return expected<int&, int&>(dummy);
    });
    REQUIRE(r.has_value());
}

// --- Monadic: transform rvalue/const-rvalue on ERROR state ---
TEST_CASE("transform rvalue on error short-circuits", "[expected_ref_both]") {
    int                  err = 3;
    expected<int&, int&> e(unexpect, err);
    auto                 r = std::move(e).transform([](int& v) { return v * 2; });
    REQUIRE(!r.has_value());
}

TEST_CASE("transform const rvalue on error short-circuits", "[expected_ref_both]") {
    int                        err = 3;
    const expected<int&, int&> e(unexpect, err);
    auto                       r = std::move(e).transform([](int& v) { return v * 2; });
    REQUIRE(!r.has_value());
}

// --- Monadic: transform_error rvalue/const-rvalue ---
TEST_CASE("transform_error rvalue on value short-circuits", "[expected_ref_both]") {
    int                  x = 42;
    expected<int&, int&> e(x);
    auto                 r = std::move(e).transform_error([](int& v) { return v * 2; });
    REQUIRE(r.has_value());
    CHECK(*r == 42);
}

TEST_CASE("transform_error const rvalue on value short-circuits", "[expected_ref_both]") {
    int                        x = 42;
    const expected<int&, int&> e(x);
    auto                       r = std::move(e).transform_error([](int& v) { return v * 2; });
    REQUIRE(r.has_value());
}

TEST_CASE("transform_error rvalue on error calls F", "[expected_ref_both]") {
    int                  err = 5;
    expected<int&, int&> e(unexpect, err);
    auto                 r = std::move(e).transform_error([](int& v) { return v + 1; });
    REQUIRE(!r.has_value());
    CHECK(r.error() == 6);
}

TEST_CASE("transform_error const rvalue on error calls F", "[expected_ref_both]") {
    int                        err = 5;
    const expected<int&, int&> e(unexpect, err);
    auto                       r = std::move(e).transform_error([](int& v) { return v + 1; });
    REQUIRE(!r.has_value());
    CHECK(r.error() == 6);
}

// --- Cross-type equality ---
TEST_CASE("equality error vs value", "[expected_ref_both]") {
    int                  x = 1, err = 0;
    expected<int&, int&> a(x);
    expected<int&, int&> b(unexpect, err);
    CHECK_FALSE(a == b);
}

// ---------------------------------------------------------------------------
// expected<T&, E&>: lvalue monadic paths
// ---------------------------------------------------------------------------

TEST_CASE("ref<T&,E&>: and_then lvalue value and error paths", "[expected_ref_both][traced]") {
    int                  x = 5, err = 3;
    expected<int&, int&> ev(x);
    auto                 rv = ev.and_then([](int& v) -> expected<int, int&> {
        static int dummy = 0;
        dummy            = v * 2;
        return dummy;
    });
    REQUIRE(rv.has_value());

    expected<int&, int&> ee(unexpect, err);
    auto                 re = ee.and_then([](int&) -> expected<int, int&> {
        static int dummy = 0;
        return expected<int, int&>(unexpect, dummy);
    });
    REQUIRE(!re.has_value());
}

TEST_CASE("ref<T&,E&>: or_else lvalue value and error paths", "[expected_ref_both][traced]") {
    int                  x = 5, err = 3;
    expected<int&, int&> ev(x);
    auto                 rv = ev.or_else([](int&) -> expected<int&, int&> {
        static int dummy = 0;
        return expected<int&, int&>(dummy);
    });
    REQUIRE(rv.has_value());

    expected<int&, int&> ee(unexpect, err);
    auto                 re = ee.or_else([](int& v) -> expected<int&, int&> {
        static int result = 0;
        result            = v * 10;
        return expected<int&, int&>(result);
    });
    REQUIRE(re.has_value());
}

TEST_CASE("ref<T&,E&>: transform lvalue value and error paths", "[expected_ref_both][traced]") {
    int                  x = 4, err = 3;
    expected<int&, int&> ev(x);
    auto                 rv = ev.transform([](int& v) { return v + 1; });
    REQUIRE(rv.has_value());
    CHECK(*rv == 5);

    expected<int&, int&> ee(unexpect, err);
    auto                 re = ee.transform([](int&) { return 0; });
    REQUIRE(!re.has_value());
}

TEST_CASE("ref<T&,E&>: transform rvalue value path", "[expected_ref_both][traced]") {
    int                  x = 4;
    expected<int&, int&> e(x);
    auto                 r = std::move(e).transform([](int& v) { return v + 1; });
    REQUIRE(r.has_value());
}

TEST_CASE("ref<T&,E&>: transform const rvalue value path", "[expected_ref_both][traced]") {
    int                        x = 4;
    const expected<int&, int&> e(x);
    auto                       r = std::move(e).transform([](int& v) { return v + 1; });
    REQUIRE(r.has_value());
}

TEST_CASE("ref<T&,E&>: transform_error lvalue value and error paths", "[expected_ref_both][traced]") {
    int                  x = 5, err = 7;
    expected<int&, int&> ev(x);
    auto                 rv = ev.transform_error([](int&) { return 0; });
    REQUIRE(rv.has_value());

    expected<int&, int&> ee(unexpect, err);
    auto                 re = ee.transform_error([](int& v) { return v + 1; });
    REQUIRE(!re.has_value());
    CHECK(re.error() == 8);
}

TEST_CASE("ref<T&,E&>: transform_error rvalue value path", "[expected_ref_both][traced]") {
    int                  x = 5;
    expected<int&, int&> e(x);
    auto                 r = std::move(e).transform_error([](int&) { return 0; });
    REQUIRE(r.has_value());
}

TEST_CASE("ref<T&,E&>: transform_error const rvalue value path", "[expected_ref_both][traced]") {
    int                        x = 5;
    const expected<int&, int&> e(x);
    auto                       r = std::move(e).transform_error([](int&) { return 0; });
    REQUIRE(r.has_value());
}

TEST_CASE("ref<T&,E&>: equality both errors same value", "[expected_ref_both][traced]") {
    int                  e1 = 1, e2 = 1;
    expected<int&, int&> a(unexpect, e1);
    expected<int&, int&> b(unexpect, e2);
    CHECK(a == b);
}
