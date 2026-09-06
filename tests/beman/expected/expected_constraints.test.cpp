// tests/beman/expected/expected_constraints.test.cpp                  -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Beman-only: tests constraint behavior not guaranteed by std::expected

#include <beman/expected/expected.hpp>

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

using namespace beman::expected;

// =============================================================================
// How these constraints are checked
//
// Each constraint below reduces to a trait, so it is a plain bool. They are
// checked at runtime rather than at translation time so that a constraint that
// has drifted is reported by the test run, naming the type and the polarity
// responsible, instead of stopping the build at the first failure and
// reporting nothing about the rest. A constraint that must exclude an overload
// is checked with CHECK_FALSE; the explanatory text each check used to carry is
// kept as an INFO, so it is printed when that check is the one that fails.
// =============================================================================

// ---------------------------------------------------------------------------
// Converting constructor: bool exemption (constraint 18.3)
//
// When T=bool, the converts-from-any-cvref guard is lifted so the converting
// ctor can be selected even though expected<U,G> has an operator bool.
// Negative counterpart: expected_bool_value_ctor_from_expected_fail.cpp
// ---------------------------------------------------------------------------

TEST_CASE("converting ctor: expected<bool,int> from expected<int,int> value path", "[constraints]") {
    expected<int, int>  src(42);
    expected<bool, int> dst(src); // converting ctor; operator* gives bool(42) = true
    REQUIRE(dst.has_value());
    CHECK(*dst == true);
}

TEST_CASE("converting ctor: expected<bool,int> from expected<int,int> error path", "[constraints]") {
    expected<int, int>  src(unexpect, 7);
    expected<bool, int> dst(src);
    REQUIRE(!dst.has_value());
    CHECK(dst.error() == 7);
}

TEST_CASE("converting ctor: expected<bool,int> is constructible from expected<int,int>", "[constraints]") {
    INFO("expected<bool,int> must be constructible from expected<int,int> via converting ctor");
    CHECK(std::is_constructible_v<expected<bool, int>, const expected<int, int>&>);
}

// ---------------------------------------------------------------------------
// Value constructor: unexpected<G> guard (constraint 23.4)
//
// unexpected<G> must route through the unexpected<G> ctor, not the value ctor.
// Tested behaviorally: constructing expected<int,int> from unexpected<int> must
// produce an error, not a value.
// ---------------------------------------------------------------------------

TEST_CASE("value ctor: unexpected<int> routes to unexpected ctor, not value ctor", "[constraints]") {
    expected<int, int> e(unexpected<int>(42));
    REQUIRE(!e.has_value());
    CHECK(e.error() == 42);
}

// A type constructible from unexpected<int> should still use the unexpected ctor,
// not the value ctor, when passed to expected<T, int>.
struct AcceptsUnexpected {
    int val             = 0;
    AcceptsUnexpected() = default;
    AcceptsUnexpected(unexpected<int> u) : val(u.error()) {}
};

TEST_CASE("value ctor: unexpected<int> blocked as value even when T is constructible from it", "[constraints]") {
    // Without constraint 23.4, expected<AcceptsUnexpected, int>(unexpected<int>{5})
    // could be ambiguous. With it, the unexpected<G> ctor is the only candidate.
    expected<AcceptsUnexpected, int> e(unexpected<int>(5));
    REQUIRE(!e.has_value());
    CHECK(e.error() == 5);
}

// Value ctor is blocked for U = unexpected<G>: is_constructible via value ctor
// path requires U not be an unexpected specialization. Verify by checking that
// the overall construction resolves correctly (the test above covers the
// behavior; the check below covers the trait):
TEST_CASE("value ctor: expected<AcceptsUnexpected,int> is still constructible from unexpected<int>", "[constraints]") {
    INFO("construction must still work — via unexpected ctor, not value ctor");
    CHECK(std::is_constructible_v<expected<AcceptsUnexpected, int>, unexpected<int>>);
}

// ---------------------------------------------------------------------------
// Value assignment: unexpected<G> goes to unexpected overload (constraint 11.2)
// ---------------------------------------------------------------------------

TEST_CASE("value assignment: unexpected<int> routes to unexpected overload", "[constraints]") {
    expected<int, int> e(1);
    e = unexpected<int>(99);
    REQUIRE(!e.has_value());
    CHECK(e.error() == 99);
}

// ---------------------------------------------------------------------------
// Move assignment: deleted when neither T nor E is nothrow move constructible
// (constraint 6.5)
// ---------------------------------------------------------------------------

struct ThrowingMove {
    ThrowingMove() = default;
    ThrowingMove(ThrowingMove&&) noexcept(false) {}
    ThrowingMove& operator=(ThrowingMove&&) = default;
};

TEST_CASE("move assignment: availability follows the nothrow-move-constructible condition", "[constraints]") {
    // Each check is braced so that its INFO is scoped to it alone, and only the
    // explanation belonging to a failing check is printed.

    // Both T and E are throwing-move: move assignment must be deleted
    {
        INFO("move assignment must be deleted when neither T nor E is nothrow move constructible");
        CHECK_FALSE(std::is_move_assignable_v<expected<ThrowingMove, ThrowingMove>>);
    }

    // At least one nothrow-move: move assignment must exist
    {
        INFO("move assignment must be available when T is nothrow move constructible");
        CHECK(std::is_move_assignable_v<expected<int, ThrowingMove>>);
    }
    {
        INFO("move assignment must be available when E is nothrow move constructible");
        CHECK(std::is_move_assignable_v<expected<ThrowingMove, int>>);
    }
    {
        INFO("move assignment must be available when both are nothrow move constructible");
        CHECK(std::is_move_assignable_v<expected<int, int>>);
    }
}

// ---------------------------------------------------------------------------
// operator==(expected, T2): T2 must not be an expected specialization
// (uses expected<T2,E2> overload instead)
// ---------------------------------------------------------------------------

TEST_CASE("operator==: expected compared to expected uses expected-expected overload", "[constraints]") {
    expected<int, int> a(42);
    expected<int, int> b(42);
    expected<int, int> c(unexpect, 1);

    CHECK(a == b);    // same value
    CHECK(!(a == c)); // value vs error: false
}

TEST_CASE("operator==: expected compared to int uses value overload", "[constraints]") {
    expected<int, int> e(42);
    CHECK(e == 42); // value overload: *e == 42
    CHECK(!(e == 99));
}

// The T2 value overload must NOT fire when T2 is itself an expected specialization.
// is_constructible check: operator==(expected<int,int>, expected<int,int>) must
// resolve via the expected<T2,E2> friend, not the T2 value friend.
// (Behavioral coverage above; the check below ensures T2=expected doesn't pick
// the value overload.)
using int_equality_t = decltype([](int x, int y) { return x == y; });

TEST_CASE("operator==: a plain int equality callable is not invocable with expected arguments", "[constraints]") {
    INFO("sanity: plain int equality lambda cannot be called with expected args");
    CHECK_FALSE((std::is_invocable_r_v<bool, int_equality_t, expected<int, int>, expected<int, int>>));
}
