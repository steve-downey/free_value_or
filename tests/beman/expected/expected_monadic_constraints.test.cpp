// tests/beman/expected/expected_monadic_constraints.test.cpp            -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Beman-only: tests SFINAE behavior of monadic operation constraints.
// The standard says these are Constraints, so they must participate in
// overload resolution (SFINAE-friendly), not produce hard errors.

#include <beman/expected/expected.hpp>

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

using namespace beman::expected;

// =============================================================================
// How these constraints are checked
//
// Every check below is the satisfaction of a detector concept, so it is a
// plain bool. They are checked at runtime rather than with static_assert so
// that a constraint that has drifted is reported by the test run, naming the
// operation and the value category responsible, instead of stopping the build
// at the first failure and reporting nothing about the rest.
//
// The polarity matters as much as the value: an operation that must be
// constrained *out* is checked with CHECK_FALSE.
// =============================================================================

// A type that is not constructible from lvalue ref (only move-constructible)
struct MoveOnly {
    MoveOnly()                           = default;
    MoveOnly(MoveOnly&&)                 = default;
    MoveOnly& operator=(MoveOnly&&)      = default;
    MoveOnly(const MoveOnly&)            = delete;
    MoveOnly& operator=(const MoveOnly&) = delete;
};

// Concept detectors for each monadic operation (use declval to preserve value category)
template <class X, class F>
concept has_and_then = requires(F f) { std::declval<X>().and_then(f); };

template <class X, class F>
concept has_or_else = requires(F f) { std::declval<X>().or_else(f); };

template <class X, class F>
concept has_transform = requires(F f) { std::declval<X>().transform(f); };

template <class X, class F>
concept has_transform_error = requires(F f) { std::declval<X>().transform_error(f); };

// ---------------------------------------------------------------------------
// Primary template: and_then / transform need E constructible from error()
// ---------------------------------------------------------------------------

// MoveOnly as E: lvalue overloads (&, const&) are constrained out because
// E is not copy-constructible, so is_constructible_v<E, E&> is false.
//
// The aliases and callables stay at namespace scope, next to the section they
// belong to: naming them there keeps each detector-concept argument short
// enough to read, and each case is then a list of the constraints alone.
using MoveOnlyErr                     = expected<int, MoveOnly>;
[[maybe_unused]] auto dummy_and_then  = [](int) { return expected<int, MoveOnly>(); };
[[maybe_unused]] auto dummy_transform = [](int) { return 0; };

TEST_CASE("monadic constraints: and_then / transform on expected<int, MoveOnly>", "[monadic][constraints]") {
    // Lvalue overloads require is_constructible_v<E, E&>, which fails for a
    // move-only E, so only the rvalue overload survives.
    CHECK_FALSE(has_and_then<MoveOnlyErr&, decltype(dummy_and_then)>);
    CHECK(has_and_then<MoveOnlyErr&&, decltype(dummy_and_then)>);
    CHECK_FALSE(has_and_then<const MoveOnlyErr&, decltype(dummy_and_then)>);

    CHECK_FALSE(has_transform<MoveOnlyErr&, decltype(dummy_transform)>);
    CHECK(has_transform<MoveOnlyErr&&, decltype(dummy_transform)>);
    CHECK_FALSE(has_transform<const MoveOnlyErr&, decltype(dummy_transform)>);
}

// ---------------------------------------------------------------------------
// Primary template: or_else / transform_error need T constructible from *this
// ---------------------------------------------------------------------------

using MoveOnlyVal                           = expected<MoveOnly, int>;
[[maybe_unused]] auto dummy_or_else         = [](int) { return expected<MoveOnly, int>(); };
[[maybe_unused]] auto dummy_transform_error = [](int) { return 0; };

TEST_CASE("monadic constraints: or_else / transform_error on expected<MoveOnly, int>", "[monadic][constraints]") {
    // Lvalue overloads require is_constructible_v<T, T&>, which fails for a
    // move-only T, so only the rvalue overload survives.
    CHECK_FALSE(has_or_else<MoveOnlyVal&, decltype(dummy_or_else)>);
    CHECK(has_or_else<MoveOnlyVal&&, decltype(dummy_or_else)>);
    CHECK_FALSE(has_or_else<const MoveOnlyVal&, decltype(dummy_or_else)>);

    CHECK_FALSE(has_transform_error<MoveOnlyVal&, decltype(dummy_transform_error)>);
    CHECK(has_transform_error<MoveOnlyVal&&, decltype(dummy_transform_error)>);
    CHECK_FALSE(has_transform_error<const MoveOnlyVal&, decltype(dummy_transform_error)>);
}

// ---------------------------------------------------------------------------
// Void specialization: and_then / transform need E constructible from error()
// ---------------------------------------------------------------------------

using VoidMoveOnlyErr                      = expected<void, MoveOnly>;
[[maybe_unused]] auto void_dummy_and_then  = []() { return expected<void, MoveOnly>(); };
[[maybe_unused]] auto void_dummy_transform = []() { return 0; };

TEST_CASE("monadic constraints: and_then / transform on expected<void, MoveOnly>", "[monadic][constraints]") {
    CHECK_FALSE(has_and_then<VoidMoveOnlyErr&, decltype(void_dummy_and_then)>);
    CHECK(has_and_then<VoidMoveOnlyErr&&, decltype(void_dummy_and_then)>);
    CHECK_FALSE(has_and_then<const VoidMoveOnlyErr&, decltype(void_dummy_and_then)>);

    CHECK_FALSE(has_transform<VoidMoveOnlyErr&, decltype(void_dummy_transform)>);
    CHECK(has_transform<VoidMoveOnlyErr&&, decltype(void_dummy_transform)>);
    CHECK_FALSE(has_transform<const VoidMoveOnlyErr&, decltype(void_dummy_transform)>);
}

// ---------------------------------------------------------------------------
// Void specialization: or_else / transform_error have NO constraints
// ---------------------------------------------------------------------------

[[maybe_unused]] auto void_dummy_or_else         = [](MoveOnly&&) { return expected<void, int>(); };
[[maybe_unused]] auto void_dummy_transform_error = [](MoveOnly&&) { return 0; };

TEST_CASE("monadic constraints: void or_else / transform_error are unconstrained", "[monadic][constraints]") {
    // Nothing is required of T, because there is no T to reconstruct, so these
    // are available even for an error type that is only move-constructible.
    CHECK(has_or_else<VoidMoveOnlyErr&&, decltype(void_dummy_or_else)>);
    CHECK(has_transform_error<VoidMoveOnlyErr&&, decltype(void_dummy_transform_error)>);
}

// ---------------------------------------------------------------------------
// Normal types: all operations remain available
// ---------------------------------------------------------------------------

using NormalExpected                       = expected<int, int>;
[[maybe_unused]] auto normal_and_then      = [](int) { return expected<int, int>(42); };
[[maybe_unused]] auto normal_or_else       = [](int) { return expected<int, int>(42); };
[[maybe_unused]] auto normal_transform     = [](int) { return 42; };
[[maybe_unused]] auto normal_transform_err = [](int) { return 42; };

TEST_CASE("monadic constraints: every operation available for expected<int, int>", "[monadic][constraints]") {
    CHECK(has_and_then<NormalExpected&, decltype(normal_and_then)>);
    CHECK(has_and_then<NormalExpected&&, decltype(normal_and_then)>);
    CHECK(has_and_then<const NormalExpected&, decltype(normal_and_then)>);
    CHECK(has_and_then<const NormalExpected&&, decltype(normal_and_then)>);

    CHECK(has_or_else<NormalExpected&, decltype(normal_or_else)>);
    CHECK(has_transform<NormalExpected&, decltype(normal_transform)>);
    CHECK(has_transform_error<NormalExpected&, decltype(normal_transform_err)>);
}

TEST_CASE("monadic constraints: rvalue and_then works with move-only error", "[monadic][constraints]") {
    expected<int, MoveOnly> e(42);
    auto                    result = std::move(e).and_then([](int v) { return expected<int, MoveOnly>(v + 1); });
    REQUIRE(result.has_value());
    CHECK(*result == 43);
}

TEST_CASE("monadic constraints: rvalue or_else works with move-only value", "[monadic][constraints]") {
    expected<MoveOnly, int> e(unexpect, 7);
    auto                    result = std::move(e).or_else([](int) { return expected<MoveOnly, int>(); });
    REQUIRE(result.has_value());
}

TEST_CASE("monadic constraints: rvalue transform works with move-only error", "[monadic][constraints]") {
    expected<int, MoveOnly> e(42);
    auto                    result = std::move(e).transform([](int v) { return v * 2; });
    REQUIRE(result.has_value());
    CHECK(*result == 84);
}

TEST_CASE("monadic constraints: rvalue transform_error works with move-only value", "[monadic][constraints]") {
    expected<MoveOnly, int> e(unexpect, 7);
    auto                    result = std::move(e).transform_error([](int v) { return v + 1; });
    REQUIRE(!result.has_value());
    CHECK(result.error() == 8);
}

TEST_CASE("monadic constraints: void and_then rvalue with move-only error", "[monadic][constraints]") {
    expected<void, MoveOnly> e;
    auto                     result = std::move(e).and_then([]() { return expected<void, MoveOnly>(); });
    REQUIRE(result.has_value());
}

TEST_CASE("monadic constraints: void transform rvalue with move-only error", "[monadic][constraints]") {
    expected<void, MoveOnly> e;
    auto                     result = std::move(e).transform([]() { return 42; });
    REQUIRE(result.has_value());
    CHECK(*result == 42);
}
