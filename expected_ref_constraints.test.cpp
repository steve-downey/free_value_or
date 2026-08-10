// tests/beman/expected/expected_ref_constraints.test.cpp             -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Beman-only: tests SFINAE behavior of constraints on expected<T&, E>.
//
// Every constraint below is checked at runtime rather than with static_assert
// so that a violated constraint is reported by the test run, naming the
// responsible type, instead of stopping the build at the first failure and
// reporting nothing at all. Each check is a boolean trait or concept, so a
// plain CHECK / CHECK_FALSE is exactly as strict as the static_assert it
// replaces; the polarity of the original is preserved.

#include <beman/expected/expected.hpp>
#include <beman/expected/expected.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <type_traits>

using namespace beman::expected;

// ---------------------------------------------------------------------------
// Helper types
// ---------------------------------------------------------------------------

struct MoveOnly {
    MoveOnly()                           = default;
    MoveOnly(MoveOnly&&)                 = default;
    MoveOnly& operator=(MoveOnly&&)      = default;
    MoveOnly(const MoveOnly&)            = delete;
    MoveOnly& operator=(const MoveOnly&) = delete;
};

struct NotSwappable {
    NotSwappable()                                   = default;
    NotSwappable(NotSwappable&&)                     = default;
    NotSwappable& operator=(NotSwappable&&)          = default;
    NotSwappable(const NotSwappable&)                = default;
    NotSwappable& operator=(const NotSwappable&)     = default;
    friend void   swap(NotSwappable&, NotSwappable&) = delete;
};

// Concept detectors
template <class X, class F>
concept has_and_then = requires(F f) { std::declval<X>().and_then(f); };

template <class X, class F>
concept has_or_else = requires(F f) { std::declval<X>().or_else(f); };

template <class X, class F>
concept has_transform = requires(F f) { std::declval<X>().transform(f); };

template <class X, class F>
concept has_transform_error = requires(F f) { std::declval<X>().transform_error(f); };

template <class X>
concept has_swap = requires(X a, X b) { a.swap(b); };

template <class X, class U>
concept has_emplace = requires(X x, U u) { x.emplace(u); };

template <class X, class U>
concept has_emplace_from = requires { std::declval<X&>().emplace(std::declval<U>()); };

template <class X, class U>
concept has_value_or = requires(X x, U u) { x.value_or(u); };

// ===========================================================================
// C2/C3: Copy/move constructors require E to be copy/move constructible
// ===========================================================================

TEST_CASE("ref constraint: copy/move ctor track E copy/move constructibility", "[ref_constraints]") {
    {
        INFO("expected<T&, MoveOnly> must not be copy-constructible");
        CHECK_FALSE((std::is_copy_constructible_v<expected<int&, MoveOnly>>));
    }
    {
        INFO("expected<T&, MoveOnly> must be move-constructible");
        CHECK((std::is_move_constructible_v<expected<int&, MoveOnly>>));
    }
    {
        INFO("expected<T&, int> must be copy-constructible");
        CHECK((std::is_copy_constructible_v<expected<int&, int>>));
    }
    {
        INFO("expected<T&, int> must be move-constructible");
        CHECK((std::is_move_constructible_v<expected<int&, int>>));
    }
}

// ===========================================================================
// A1/A2: Copy/move assignment require E to be copy/move constructible+assignable
// ===========================================================================

TEST_CASE("ref constraint: copy/move assignment track E copy/move assignability", "[ref_constraints]") {
    {
        INFO("expected<T&, MoveOnly> must not be copy-assignable");
        CHECK_FALSE((std::is_copy_assignable_v<expected<int&, MoveOnly>>));
    }
    {
        INFO("expected<T&, MoveOnly> must be move-assignable");
        CHECK((std::is_move_assignable_v<expected<int&, MoveOnly>>));
    }
    {
        INFO("expected<T&, int> must be copy-assignable");
        CHECK((std::is_copy_assignable_v<expected<int&, int>>));
    }
    {
        INFO("expected<T&, int> must be move-assignable");
        CHECK((std::is_move_assignable_v<expected<int&, int>>));
    }
}

// ===========================================================================
// C5/C6: Value ctor excludes expected self-type and unexpected specializations
// ===========================================================================

// unexpected<int> must route to the unexpected ctor, not the value ctor.
// Verify by constructing — if the value ctor were selected, it would try to
// bind int& to an unexpected<int>, which would fail differently.
TEST_CASE("ref constraint: unexpected routes to unexpected ctor", "[ref_constraints]") {
    expected<int&, int> e(unexpected<int>(42));
    REQUIRE(!e.has_value());
    CHECK(e.error() == 42);
}

// ===========================================================================
// C7: Value ctor requires is_constructible_v<T&, U>
// ===========================================================================

TEST_CASE("ref constraint: value ctor requires is_constructible_v<T&, U>", "[ref_constraints]") {
    // Cannot construct expected<int&, int> from a string — int& is not
    // bindable to string
    {
        INFO("expected<int&, int> must not be constructible from string&");
        CHECK_FALSE((std::is_constructible_v<expected<int&, int>, std::string&>));
    }
    // Can construct expected<int&, int> from int&
    {
        INFO("expected<int&, int> must be constructible from int&");
        CHECK((std::is_constructible_v<expected<int&, int>, int&>));
    }
}

// ===========================================================================
// C9/C10: Converting ctor from expected<U&, G>
// ===========================================================================

// Cannot convert when E is not constructible from G
struct Unconstructible {
    Unconstructible()    = default;
    Unconstructible(int) = delete;
};

TEST_CASE("ref constraint: converting ctor from expected<U&, G>", "[ref_constraints]") {
    // Cannot convert expected<string&, int> to expected<int&, int> (string&
    // not bindable to int&)
    {
        INFO("expected<int&, int> must not be constructible from expected<string&, int>");
        CHECK_FALSE((std::is_constructible_v<expected<int&, int>, const expected<std::string&, int>&>));
    }
    // Can convert expected<int&, int> to expected<int&, int> (same types)
    {
        INFO("expected<int&, int> must be constructible from expected<int&, int>");
        CHECK((std::is_constructible_v<expected<int&, int>, const expected<int&, int>&>));
    }
    {
        INFO("expected<int&, Unconstructible> not constructible from expected<int&, int> — E(const G&) fails");
        CHECK_FALSE((std::is_constructible_v<expected<int&, Unconstructible>, const expected<int&, int>&>));
    }
}

// ===========================================================================
// A3/A4/A5: Value assignment constraints
// ===========================================================================

TEST_CASE("ref constraint: value assignment constraints", "[ref_constraints]") {
    // Value assignment from int& works
    {
        INFO("expected<int&, int> must be assignable from int&");
        CHECK((std::is_assignable_v<expected<int&, int>&, int&>));
    }
    // Value assignment from unrelated type that can't bind to int& is blocked
    {
        INFO("expected<int&, int> must not be assignable from string&");
        CHECK_FALSE((std::is_assignable_v<expected<int&, int>&, std::string&>));
    }
}

// ===========================================================================
// A6: unexpected<G> assignment requires constructible+assignable E
// ===========================================================================

TEST_CASE("ref constraint: unexpected<G> assignment requires constructible+assignable E", "[ref_constraints]") {
    INFO("expected<int&, int> must be assignable from unexpected<int>");
    CHECK((std::is_assignable_v<expected<int&, int>&, unexpected<int>>));
}

// ===========================================================================
// S1: swap requires E swappable and move-constructible
// ===========================================================================

TEST_CASE("ref constraint: swap requires E swappable and move-constructible", "[ref_constraints]") {
    {
        INFO("expected<int&, int> must be swappable");
        CHECK((has_swap<expected<int&, int>>));
    }
    {
        INFO("expected<int&, NotSwappable> must not be swappable");
        CHECK_FALSE((has_swap<expected<int&, NotSwappable>>));
    }
}

// ===========================================================================
// E1: emplace requires constructible and no dangling
// ===========================================================================

TEST_CASE("ref constraint: emplace requires constructible and no dangling", "[ref_constraints]") {
    {
        INFO("expected<int&, int> must support emplace from int&");
        CHECK((has_emplace<expected<int&, int>, int&>));
    }
    // emplace from an rvalue int — should be blocked (can't bind int& to rvalue)
    {
        INFO("expected<int&, int> must not support emplace from rvalue int");
        CHECK_FALSE((has_emplace_from<expected<int&, int>, int>));
    }
}

// ===========================================================================
// V1: value_or requires is_object_v<T> && !is_array_v<T> (LWG4304)
// ===========================================================================

TEST_CASE("ref constraint: value_or requires object type (LWG4304)", "[ref_constraints]") {
    INFO("expected<int&, int> must support value_or");
    CHECK((has_value_or<const expected<int&, int>, int>));

    // Function type: int(int) is not an object type
    // Note: expected<int(&)(int), E> would require T = int(int), which is a function type.
    // We can't easily form expected<FuncRef, E> due to other constraints, so we verify
    // the positive case works and trust the requires clause.
}

// ===========================================================================
// MC1/MC2: and_then/transform constrained by E constructibility
// ===========================================================================

using RefMoveOnlyErr                     = expected<int&, MoveOnly>;
[[maybe_unused]] auto ref_dummy_and_then = [](int&) { return expected<int, MoveOnly>(); };

[[maybe_unused]] auto ref_dummy_transform = [](int&) { return 0; };

TEST_CASE("ref constraint: and_then/transform constrained by E constructibility", "[ref_constraints]") {
    {
        INFO("and_then lvalue must be constrained out when E is move-only");
        CHECK_FALSE((has_and_then<RefMoveOnlyErr&, decltype(ref_dummy_and_then)>));
    }
    {
        INFO("and_then rvalue must be available when E is move-constructible");
        CHECK((has_and_then<RefMoveOnlyErr&&, decltype(ref_dummy_and_then)>));
    }
    {
        INFO("and_then const lvalue must be constrained out when E is move-only");
        CHECK_FALSE((has_and_then<const RefMoveOnlyErr&, decltype(ref_dummy_and_then)>));
    }
    {
        INFO("transform lvalue must be constrained out when E is move-only");
        CHECK_FALSE((has_transform<RefMoveOnlyErr&, decltype(ref_dummy_transform)>));
    }
    {
        INFO("transform rvalue must be available when E is move-constructible");
        CHECK((has_transform<RefMoveOnlyErr&&, decltype(ref_dummy_transform)>));
    }
    {
        INFO("transform const lvalue must be constrained out when E is move-only");
        CHECK_FALSE((has_transform<const RefMoveOnlyErr&, decltype(ref_dummy_transform)>));
    }
}

// ===========================================================================
// MC3/MC4: or_else/transform_error have no constraints — always available
// ===========================================================================

[[maybe_unused]] auto ref_dummy_or_else = [](MoveOnly&&) -> expected<int&, int> {
    static int x = 0;
    return expected<int&, int>(x);
};

[[maybe_unused]] auto ref_dummy_transform_error = [](MoveOnly&&) { return 0; };

TEST_CASE("ref constraint: or_else/transform_error are unconstrained", "[ref_constraints]") {
    {
        INFO("or_else must be available — no constraints on value constructibility for T&");
        CHECK((has_or_else<RefMoveOnlyErr&&, decltype(ref_dummy_or_else)>));
    }
    {
        INFO("transform_error must be available — no constraints for T& specialization");
        CHECK((has_transform_error<RefMoveOnlyErr&&, decltype(ref_dummy_transform_error)>));
    }
}

// ===========================================================================
// Positive: all operations available for normal types
// ===========================================================================

using NormalRef = expected<int&, int>;

[[maybe_unused]] auto ref_normal_and_then = [](int&) { return expected<int, int>(42); };
[[maybe_unused]] auto ref_normal_or_else  = [](int) -> expected<int&, int> {
    static int x = 0;
    return expected<int&, int>(x);
};
[[maybe_unused]] auto ref_normal_transform     = [](int&) { return 42; };
[[maybe_unused]] auto ref_normal_transform_err = [](int) { return 42; };

TEST_CASE("ref constraint: all monadic operations available for normal types", "[ref_constraints]") {
    CHECK((has_and_then<NormalRef&, decltype(ref_normal_and_then)>));
    CHECK((has_and_then<NormalRef&&, decltype(ref_normal_and_then)>));
    CHECK((has_and_then<const NormalRef&, decltype(ref_normal_and_then)>));
    CHECK((has_and_then<const NormalRef&&, decltype(ref_normal_and_then)>));

    CHECK((has_or_else<NormalRef&, decltype(ref_normal_or_else)>));
    CHECK((has_transform<NormalRef&, decltype(ref_normal_transform)>));
    CHECK((has_transform_error<NormalRef&, decltype(ref_normal_transform_err)>));
}

// ===========================================================================
// Runtime tests for constraint-gated paths
// ===========================================================================

TEST_CASE("ref constraint: rvalue and_then works with move-only error", "[ref_constraints]") {
    int                      x = 42;
    expected<int&, MoveOnly> e(x);
    auto                     r = std::move(e).and_then([](int& v) { return expected<int, MoveOnly>(v + 1); });
    REQUIRE(r.has_value());
    CHECK(*r == 43);
}

TEST_CASE("ref constraint: rvalue transform works with move-only error", "[ref_constraints]") {
    int                      x = 42;
    expected<int&, MoveOnly> e(x);
    auto                     r = std::move(e).transform([](int& v) { return v * 2; });
    REQUIRE(r.has_value());
    CHECK(*r == 84);
}
