// tests/beman/expected/expected_review_corrections.test.cpp            -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Regression tests for the PR #57 review corrections (findings F3-F7).
// F1/F2/F8 were addressed separately; the construction rule below is the
// Option-B resolution of F6 (accommodate the safe unexpected<E&> use case).

#include <beman/expected/expected.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <type_traits>

using namespace beman::expected;

// =============================================================================
// F6 (Option B) — construction from unexpected<G> for reference E.
//
// Allowed when G is itself a reference (the source holds a reference to an
// external object, so binding E& cannot dangle). Deleted when G is a value
// type (referent lives inside the wrapper) or would drop const.
// =============================================================================

// Checked at runtime rather than with static_assert so that a violated rule is
// reported by the test run, with the offending specialization named, instead of
// stopping the build at the first failure and reporting nothing.
TEST_CASE("F6: construction from unexpected<G> is allowed only for reference G", "[ref][unexpected]") {
    // Allowed: reference G, across all three specializations.
    CHECK(std::is_constructible_v<expected<void, int&>, unexpected<int&>>);
    CHECK(std::is_constructible_v<expected<int, int&>, unexpected<int&>>);
    CHECK(std::is_constructible_v<expected<int&, int&>, unexpected<int&>>);

    // Allowed: binding a more-const reference (const int& <- int&).
    CHECK(std::is_constructible_v<expected<void, const int&>, unexpected<int&>>);
    CHECK(std::is_constructible_v<expected<int, const int&>, unexpected<int&>>);

    // Deleted: value G would dangle once a temporary unexpected is destroyed.
    CHECK_FALSE(std::is_constructible_v<expected<void, int&>, unexpected<int>>);
    CHECK_FALSE(std::is_constructible_v<expected<int, int&>, unexpected<int>>);
    CHECK_FALSE(std::is_constructible_v<expected<int&, int&>, unexpected<int>>);

    // Deleted: dropping const (int& <- const int&) is not constructible.
    CHECK_FALSE(std::is_constructible_v<expected<void, int&>, unexpected<const int&>>);
    CHECK_FALSE(std::is_constructible_v<expected<int, int&>, unexpected<const int&>>);

    // Value-E construction is unaffected.
    CHECK(std::is_constructible_v<expected<int, std::string>, unexpected<std::string>>);
}

TEST_CASE("reference-error construction from unexpected<E&> binds an external object", "[ref][unexpected]") {
    int err = 41;

    // A temporary unexpected<int&> is fine: it only holds a pointer to err.
    expected<void, int&> a{unexpected<int&>(err)};
    expected<int, int&>  b{unexpected<int&>(err)};
    expected<int&, int&> c{unexpected<int&>(err)};

    REQUIRE(&a.error() == &err);
    REQUIRE(&b.error() == &err);
    REQUIRE(&c.error() == &err);

    err = 99; // mutating the external object is visible through the stored reference
    REQUIRE(a.error() == 99);
    REQUIRE(b.error() == 99);
    REQUIRE(c.error() == 99);
}

// Rebinding assignment from unexpected<E&> — allowed for reference E only when G is a reference
// (rebinds the error pointer to an external object; never dangles). Value G and const-drop are
// rejected at compile time, mirroring construction.
TEST_CASE("F6: rebinding assignment from unexpected<G> is allowed only for reference G", "[ref][unexpected][assign]") {
    CHECK(std::is_assignable_v<expected<void, int&>&, unexpected<int&>>);
    CHECK(std::is_assignable_v<expected<int, int&>&, unexpected<int&>>);
    CHECK(std::is_assignable_v<expected<int&, int&>&, unexpected<int&>>);
    CHECK(std::is_assignable_v<expected<int, const int&>&, unexpected<int&>>);       // more-const OK
    CHECK_FALSE(std::is_assignable_v<expected<int, int&>&, unexpected<int>>);        // value G: deleted
    CHECK_FALSE(std::is_assignable_v<expected<int, int&>&, unexpected<const int&>>); // const drop: no overload
}

TEST_CASE("rebinding assignment from unexpected<E&> repoints the error reference", "[ref][unexpected][assign]") {
    int g1 = 1, g2 = 2;

    SECTION("error -> error rebind (expected<int, int&>)") {
        expected<int, int&> e{unexpect, g1};
        e = unexpected<int&>(g2);
        REQUIRE(&e.error() == &g2); // rebound to g2
        REQUIRE(g1 == 1);           // previously-referenced object untouched
        g2 = 42;
        REQUIRE(e.error() == 42); // sees the new referent
    }
    SECTION("value -> error transition (expected<int, int&>)") {
        expected<int, int&> e{7};
        e = unexpected<int&>(g2);
        REQUIRE_FALSE(e.has_value());
        REQUIRE(&e.error() == &g2);
    }
    SECTION("both references (expected<int&, int&>)") {
        int                  target = 5;
        expected<int&, int&> e{target};
        e = unexpected<int&>(g2);
        REQUIRE_FALSE(e.has_value());
        REQUIRE(&e.error() == &g2);
    }
    SECTION("void value (expected<void, int&>)") {
        expected<void, int&> e{}; // value state
        e = unexpected<int&>(g2);
        REQUIRE_FALSE(e.has_value());
        REQUIRE(&e.error() == &g2);
    }
}

// =============================================================================
// F4 / F5 — value constructor and emplace are noexcept only when the reference
// bind cannot throw, so a throwing conversion propagates instead of terminating.
// =============================================================================

namespace {
struct ThrowingRef {
    operator int&() const { throw 42; }
};
} // namespace

TEST_CASE("F4/F5: value construction is noexcept only when the reference bind cannot throw", "[ref][noexcept]") {
    CHECK(std::is_nothrow_constructible_v<expected<int&, int>, int&>);
    CHECK_FALSE(std::is_nothrow_constructible_v<expected<int&, int>, ThrowingRef>);
}

TEST_CASE("value constructor propagates a throwing reference conversion", "[ref][noexcept]") {
    int caught = 0;
    try {
        expected<int&, int> e{ThrowingRef{}};
        (void)e;
    } catch (int v) {
        caught = v;
    }
    REQUIRE(caught == 42);
}

TEST_CASE("emplace propagates a throwing reference conversion", "[ref][emplace][noexcept]") {
    int                 target = 5;
    expected<int&, int> e{target};
    int                 caught = 0;
    try {
        e.emplace(ThrowingRef{});
    } catch (int v) {
        caught = v;
    }
    REQUIRE(caught == 42);
    REQUIRE(e.has_value()); // still holds the original value; emplace did not corrupt state
    REQUIRE(&e.value() == &target);
}

// =============================================================================
// F3 — assignment that rebinds through a throwing conversion must leave the
// error state intact and destroy the error exactly once (no double-destroy).
// =============================================================================

namespace {
int g_dtor_count = 0;
struct CountingErr {
    int value;
    explicit CountingErr(int v) : value(v) {}
    CountingErr(const CountingErr&) = default;
    ~CountingErr() { ++g_dtor_count; }
};
} // namespace

TEST_CASE("throwing rebind assignment does not double-destroy the error", "[ref][assign][exception-safety]") {
    CountingErr seed{7};
    int         caught = 0;
    {
        expected<int&, CountingErr> e{unexpect, seed};
        g_dtor_count = 0; // count only destructions of e's stored error from here on
        try {
            e = ThrowingRef{}; // was-error branch: rebind throws
        } catch (int v) {
            caught = v;
        }
        REQUIRE(caught == 42);
        REQUIRE_FALSE(e.has_value()); // error state preserved
        REQUIRE(e.error().value == 7);
        REQUIRE(g_dtor_count == 0); // not destroyed yet
    }
    REQUIRE(g_dtor_count == 1); // destroyed exactly once, at end of scope
}

// =============================================================================
// F7 — error_or is SFINAE-friendly (constraints in a requires-clause, not a
// runtime static_assert).
// =============================================================================

namespace {
template <class Ex, class Arg>
concept has_error_or = requires(Ex e, Arg a) { e.error_or(a); };
struct NotConvertible {};
} // namespace

TEST_CASE("F7: error_or is SFINAE-friendly", "[ref][error_or]") {
    CHECK(has_error_or<expected<int&, std::string>&, const char*>);
    CHECK_FALSE(has_error_or<expected<int&, std::string>&, NotConvertible>);
}

// =============================================================================
// Shallow conversion — converting from an rvalue reference-holding expected to a
// value-holding expected must NOT move-steal the external referent (the value
// category comes from the shallow accessor, via *std::move(rhs), not a naked
// std::move of the referent). Mirrors optional<T&>'s behavior.
// =============================================================================

TEST_CASE("converting from an rvalue reference expected does not steal the referent", "[ref][convert][shallow]") {
    SECTION("value side: expected<string,int> <- expected<string&,int>&&") {
        std::string                 s = "bar";
        expected<std::string&, int> r{s};
        expected<std::string, int>  o{std::move(r)};
        REQUIRE(s == "bar"); // referent untouched (copied, not moved)
        REQUIRE(o.value() == "bar");
    }
    SECTION("value side, assignment") {
        std::string                 s = "bar";
        expected<std::string&, int> r{s};
        expected<std::string, int>  o{std::in_place, "x"};
        o = std::move(r);
        REQUIRE(s == "bar");
        REQUIRE(o.value() == "bar");
    }
    SECTION("error side: expected<int,string> <- expected<int,string&>&&") {
        std::string                 s = "bar";
        expected<int, std::string&> r{unexpect, s};
        expected<int, std::string>  o{std::move(r)};
        REQUIRE(s == "bar");
        REQUIRE(o.error() == "bar");
    }
}

TEST_CASE("constructing/assigning a value error from unexpected<E&> does not steal the referent",
          "[unexpected][convert][shallow]") {
    SECTION("construction") {
        std::string                s = "bar";
        expected<int, std::string> o{unexpected<std::string&>(s)};
        REQUIRE(s == "bar"); // external referent copied, not moved
        REQUIRE(o.error() == "bar");
    }
    SECTION("assignment") {
        std::string                s = "bar";
        expected<int, std::string> o{unexpect, "x"};
        o = unexpected<std::string&>(s);
        REQUIRE(s == "bar");
        REQUIRE(o.error() == "bar");
    }
}

TEST_CASE("constructing a value error from an rvalue unexpected<E> still moves", "[unexpected][move]") {
    unexpected<std::string>    u{"baz"};
    expected<int, std::string> o{std::move(u)};
    REQUIRE(u.error().empty()); // owned error genuinely moved out
    REQUIRE(o.error() == "baz");
}

TEST_CASE("converting from an rvalue value expected still moves", "[convert][move]") {
    SECTION("value side") {
        expected<std::string, int> a{std::in_place, "bar"};
        expected<std::string, int> b{std::move(a)};
        REQUIRE(a.value().empty()); // genuinely moved-from
        REQUIRE(b.value() == "bar");
    }
    SECTION("error side") {
        expected<int, std::string> a{unexpect, "baz"};
        expected<int, std::string> b{std::move(a)};
        REQUIRE(a.error().empty());
        REQUIRE(b.error() == "baz");
    }
}
