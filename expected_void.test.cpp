// tests/beman/expected/expected_void.test.cpp                         -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/expected/expected.hpp>
#include <beman/expected/expected.hpp>  // idempotence check

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <type_traits>
#include <utility>
#include <vector>

using beman::expected::bad_expected_access;
using beman::expected::expected;
using beman::expected::unexpected;
using beman::expected::unexpect;

// =============================================================================
// [expected.void.general] Ill-formed instantiation constraints
// =============================================================================
// E must satisfy the same constraints as for unexpected<E>:
// not a reference, not an array, not cv-qualified, not unexpected<X>.
// These are enforced by static_asserts inside the class body; verified by
// negative compile tests (expected_void_ref_fail.cpp, expected_void_array_fail.cpp).

// =============================================================================
// [expected.void.cons] Constructors
// =============================================================================

// --- Default constructor ---

TEST_CASE("expected<void>: default construct", "[expected_void]") {
    expected<void, int> e;
    CHECK(e.has_value());
    static_assert(std::is_nothrow_default_constructible_v<expected<void, int>>);
}

// --- Copy constructor ---

struct NoCopyE {
    NoCopyE(const NoCopyE&) = delete;
    NoCopyE()               = default;
};

static_assert(!std::is_copy_constructible_v<expected<void, NoCopyE>>);
static_assert(std::is_trivially_copy_constructible_v<expected<void, int>>);

TEST_CASE("expected<void>: copy construct with value", "[expected_void]") {
    expected<void, int> a;
    expected<void, int> b = a;
    CHECK(b.has_value());
}

TEST_CASE("expected<void>: copy construct with error", "[expected_void]") {
    expected<void, int> a(unexpect, 42);
    expected<void, int> b = a;
    REQUIRE(!b.has_value());
    CHECK(b.error() == 42);
}

// --- Move constructor ---

static_assert(std::is_nothrow_move_constructible_v<expected<void, int>>);

TEST_CASE("expected<void>: move construct with error", "[expected_void]") {
    expected<void, std::string> a(unexpect, "err");
    expected<void, std::string> b = std::move(a);
    REQUIRE(!b.has_value());
    CHECK(b.error() == "err");
}

// --- Converting constructor from expected<U, G> where is_void_v<U> ---

// Constraint: U must be void — cannot convert from expected<int, G>
static_assert(!std::is_constructible_v<expected<void, int>, expected<int, int>>);

TEST_CASE("expected<void>: convert from expected<void, G> with value", "[expected_void]") {
    expected<void, int> src;
    expected<void, long> dst = src;
    CHECK(dst.has_value());
}

TEST_CASE("expected<void>: convert from expected<void, G> with error", "[expected_void]") {
    expected<void, int> src(unexpect, 7);
    expected<void, long> dst = src;
    REQUIRE(!dst.has_value());
    CHECK(dst.error() == 7L);
}

// --- Constructor from unexpected<G> ---

TEST_CASE("expected<void>: construct from unexpected const&", "[expected_void]") {
    expected<void, std::string> e = unexpected<std::string>("fail");
    REQUIRE(!e.has_value());
    CHECK(e.error() == "fail");
}

TEST_CASE("expected<void>: construct from unexpected&&", "[expected_void]") {
    expected<void, std::string> e = unexpected<std::string>("moved");
    REQUIRE(!e.has_value());
    CHECK(e.error() == "moved");
}

// --- in_place_t constructor ---

TEST_CASE("expected<void>: in_place_t constructor", "[expected_void]") {
    expected<void, int> e(std::in_place);
    CHECK(e.has_value());
    static_assert(noexcept(expected<void, int>(std::in_place)));
}

// --- unexpect_t constructors ---

TEST_CASE("expected<void>: unexpect_t constructor", "[expected_void]") {
    expected<void, std::string> e(unexpect, 3, 'x');
    REQUIRE(!e.has_value());
    CHECK(e.error() == "xxx");
}

TEST_CASE("expected<void>: unexpect_t ilist constructor", "[expected_void]") {
    expected<void, std::vector<int>> e(unexpect, {1, 2, 3});
    REQUIRE(!e.has_value());
    CHECK(e.error().size() == 3);
}

// =============================================================================
// [expected.void.dtor] Destructor
// =============================================================================

static_assert(std::is_trivially_destructible_v<expected<void, int>>);

TEST_CASE("expected<void>: destructor destroys error", "[expected_void]") {
    int destroyed = 0;
    struct Counted {
        int* d;
        ~Counted() { ++*d; }
    };
    {
        expected<void, Counted> e(unexpect, Counted{&destroyed});
        (void)e;
    }
    CHECK(destroyed >= 1);
}

// =============================================================================
// [expected.void.assign] Assignment
// =============================================================================

// --- Copy assignment ---

TEST_CASE("expected<void>: copy assign value-to-value (no-op)", "[expected_void]") {
    expected<void, int> a, b;
    a = b;
    CHECK(a.has_value());
}

TEST_CASE("expected<void>: copy assign error-to-value", "[expected_void]") {
    expected<void, int> a;
    expected<void, int> b(unexpect, 5);
    a = b;
    REQUIRE(!a.has_value());
    CHECK(a.error() == 5);
}

TEST_CASE("expected<void>: copy assign value-to-error", "[expected_void]") {
    expected<void, int> a(unexpect, 1), b;
    a = b;
    CHECK(a.has_value());
}

TEST_CASE("expected<void>: copy assign error-to-error", "[expected_void]") {
    expected<void, int> a(unexpect, 1), b(unexpect, 2);
    a = b;
    CHECK(a.error() == 2);
}

// --- Move assignment ---

static_assert(std::is_nothrow_move_assignable_v<expected<void, int>>);

TEST_CASE("expected<void>: move assign value-to-error", "[expected_void]") {
    expected<void, std::string> a, b(unexpect, "err");
    a = std::move(b);
    REQUIRE(!a.has_value());
    CHECK(a.error() == "err");
}

// --- Assign from unexpected<G> ---

TEST_CASE("expected<void>: assign from unexpected when value", "[expected_void]") {
    expected<void, int> e;
    e = unexpected(42);
    REQUIRE(!e.has_value());
    CHECK(e.error() == 42);
}

TEST_CASE("expected<void>: assign from unexpected when already error", "[expected_void]") {
    expected<void, int> e(unexpect, 1);
    e = unexpected(2);
    CHECK(e.error() == 2);
}

// --- emplace() ---

TEST_CASE("expected<void>: emplace from error state", "[expected_void]") {
    expected<void, int> e(unexpect, 5);
    e.emplace();
    CHECK(e.has_value());
    static_assert(noexcept(e.emplace()));
}

TEST_CASE("expected<void>: emplace from value state (no-op)", "[expected_void]") {
    expected<void, int> e;
    e.emplace();
    CHECK(e.has_value());
}

// =============================================================================
// [expected.void.swap] Swap
// =============================================================================

TEST_CASE("expected<void>: swap value-value (no-op)", "[expected_void]") {
    expected<void, int> a, b;
    a.swap(b);
    CHECK(a.has_value());
    CHECK(b.has_value());
}

TEST_CASE("expected<void>: swap value-error", "[expected_void]") {
    expected<void, int> a, b(unexpect, 7);
    a.swap(b);
    REQUIRE(!a.has_value());
    REQUIRE(b.has_value());
    CHECK(a.error() == 7);
}

TEST_CASE("expected<void>: swap error-value (reverse)", "[expected_void]") {
    expected<void, int> a(unexpect, 7), b;
    a.swap(b);
    REQUIRE(a.has_value());
    REQUIRE(!b.has_value());
    CHECK(b.error() == 7);
}

TEST_CASE("expected<void>: swap error-error", "[expected_void]") {
    expected<void, int> a(unexpect, 1), b(unexpect, 2);
    a.swap(b);
    CHECK(a.error() == 2);
    CHECK(b.error() == 1);
}

// =============================================================================
// [expected.void.obs] Observers
// =============================================================================

// --- has_value / operator bool ---

TEST_CASE("expected<void>: has_value and bool", "[expected_void]") {
    expected<void, int> a, b(unexpect, 0);
    CHECK(a.has_value());
    CHECK(bool(a));
    CHECK(!b.has_value());
    CHECK(!bool(b));
}

// --- operator*() --- returns void, noexcept

TEST_CASE("expected<void>: operator* is void", "[expected_void]") {
    expected<void, int> e;
    static_assert(std::is_same_v<decltype(*e), void>);
    *e;  // compiles and does nothing
}

// No operator-> or value_or for void (verified by negative compile tests)

// --- value() ---

TEST_CASE("expected<void>: value() on success is no-op", "[expected_void]") {
    expected<void, int> e;
    e.value();  // should not throw
}

TEST_CASE("expected<void>: value() throws on error (const&)", "[expected_void]") {
    expected<void, int> e(unexpect, 7);
    REQUIRE_THROWS_AS(e.value(), bad_expected_access<int>);
}

TEST_CASE("expected<void>: rvalue value() throws on error", "[expected_void]") {
    expected<void, int> e(unexpect, 5);
    REQUIRE_THROWS_AS(std::move(e).value(), bad_expected_access<int>);
}

// --- error() ---

TEST_CASE("expected<void>: error() all ref qualifications", "[expected_void]") {
    expected<void, int> e(unexpect, 99);
    static_assert(std::is_same_v<decltype(e.error()), int&>);
    static_assert(std::is_same_v<decltype(std::as_const(e).error()), const int&>);
    static_assert(std::is_same_v<decltype(std::move(e).error()), int&&>);
    CHECK(e.error() == 99);
}

// --- error_or() ---

TEST_CASE("expected<void>: error_or with value", "[expected_void]") {
    expected<void, int> e;
    CHECK(e.error_or(99) == 99);
}

TEST_CASE("expected<void>: error_or with error", "[expected_void]") {
    expected<void, int> e(unexpect, 7);
    CHECK(e.error_or(0) == 7);
}

// =============================================================================
// [expected.void.eq] Equality operators
// =============================================================================

TEST_CASE("expected<void>: equality both value", "[expected_void]") {
    expected<void, int> a, b;
    CHECK(a == b);
}

TEST_CASE("expected<void>: equality both error same value", "[expected_void]") {
    expected<void, int> a(unexpect, 1), b(unexpect, 1);
    CHECK(a == b);
}

TEST_CASE("expected<void>: equality both error different value", "[expected_void]") {
    expected<void, int> a(unexpect, 1), b(unexpect, 2);
    CHECK(!(a == b));
}

TEST_CASE("expected<void>: equality mixed value and error", "[expected_void]") {
    expected<void, int> a, b(unexpect, 0);
    CHECK(!(a == b));
}

TEST_CASE("expected<void>: equality with unexpected", "[expected_void]") {
    expected<void, int> a(unexpect, 3), b;
    CHECK(a == unexpected(3));
    CHECK(!(b == unexpected(3)));
}

// Cross-type equality (expected<void, int> vs expected<void, long>)
TEST_CASE("expected<void>: cross-type equality", "[expected_void]") {
    expected<void, int>  a;
    expected<void, long> b;
    CHECK(a == b);

    expected<void, int>  c(unexpect, 5);
    expected<void, long> d(unexpect, 5L);
    CHECK(c == d);
}
