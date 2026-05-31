// beman/expected/expected.test.cpp                                 -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/expected/expected.hpp>
#include <beman/expected/expected.hpp> // ensure idempotent header

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace expt = beman::expected;

// =============================================================================
// Helper types at namespace scope (needed for static_assert outside functions)
// =============================================================================

struct NoDefault {
    explicit NoDefault(int) {}
};

struct NoCopy {
    NoCopy()                         = default;
    NoCopy(const NoCopy&)            = delete;
    NoCopy(NoCopy&&)                 = default;
    NoCopy& operator=(const NoCopy&) = delete;
    NoCopy& operator=(NoCopy&&)      = default;
    int     v                        = 0;
};

struct ThrowingMove {
    ThrowingMove() = default;
    ThrowingMove(ThrowingMove&&) noexcept(false) {}
};

struct MightThrow {
    explicit MightThrow(int) noexcept(false) {}
};

// =============================================================================
// [expected.object.general] para 2-3 — type-level static assertions
// =============================================================================

// Ill-formed T: reference type — tested via negative compile file expected_t_ref_fail.cpp
// Ill-formed E: reference type — tested via negative compile file expected_e_ref_fail.cpp
// Ill-formed T: array type   — tested via negative compile file expected_t_array_fail.cpp

// Default constructor: requires is_default_constructible_v<T>
static_assert(!std::is_default_constructible_v<expt::expected<NoDefault, int>>);

// Copy constructor: not present when T is not copy-constructible
static_assert(!std::is_copy_constructible_v<expt::expected<NoCopy, int>>);

// Destructor: trivially destructible when T and E are
static_assert(std::is_trivially_destructible_v<expt::expected<int, int>>);

// Move constructor: noexcept when T and E are nothrow-move-constructible
static_assert(std::is_nothrow_move_constructible_v<expt::expected<int, int>>);
static_assert(!std::is_nothrow_move_constructible_v<expt::expected<ThrowingMove, int>>);

// Move assignment: noexcept when all four noexcept conditions hold
static_assert(std::is_nothrow_move_assignable_v<expt::expected<int, int>>);

// operator* ref-qualification return types
static_assert(std::is_same_v<decltype(*std::declval<expt::expected<int, int>&>()), int&>);
static_assert(std::is_same_v<decltype(*std::declval<const expt::expected<int, int>&>()), const int&>);
static_assert(std::is_same_v<decltype(*std::declval<expt::expected<int, int>&&>()), int&&>);
static_assert(std::is_same_v<decltype(*std::declval<const expt::expected<int, int>&&>()), const int&&>);

// error() ref-qualification return types
static_assert(std::is_same_v<decltype(std::declval<expt::expected<int, int>&>().error()), int&>);
static_assert(std::is_same_v<decltype(std::declval<const expt::expected<int, int>&>().error()), const int&>);
static_assert(std::is_same_v<decltype(std::declval<expt::expected<int, int>&&>().error()), int&&>);
static_assert(std::is_same_v<decltype(std::declval<const expt::expected<int, int>&&>().error()), const int&&>);

// =============================================================================
// Type aliases
// =============================================================================

TEST_CASE("expected: type aliases", "[ExpectedTest]") {
    static_assert(std::is_same_v<expt::expected<int, std::string>::value_type, int>);
    static_assert(std::is_same_v<expt::expected<int, std::string>::error_type, std::string>);
    static_assert(std::is_same_v<expt::expected<int, std::string>::unexpected_type, expt::unexpected<std::string>>);
    static_assert(
        std::is_same_v<expt::expected<int, std::string>::rebind<double>, expt::expected<double, std::string>>);
}

// =============================================================================
// Default construction
// =============================================================================

TEST_CASE("expected: default construction has value", "[ExpectedTest]") {
    expt::expected<int, std::string> e;
    CHECK(e.has_value());
    CHECK(static_cast<bool>(e));
    CHECK(*e == 0); // int is value-initialized
}

TEST_CASE("expected: default construction of string", "[ExpectedTest]") {
    expt::expected<std::string, int> e;
    CHECK(e.has_value());
    CHECK(*e == "");
}

// =============================================================================
// Construction from value
// =============================================================================

TEST_CASE("expected: construct from int value", "[ExpectedTest]") {
    expt::expected<int, std::string> e(42);
    CHECK(e.has_value());
    CHECK(*e == 42);
}

TEST_CASE("expected: construct from string value", "[ExpectedTest]") {
    expt::expected<std::string, int> e("hello");
    CHECK(e.has_value());
    CHECK(*e == "hello");
}

TEST_CASE("expected: construct from value by copy", "[ExpectedTest]") {
    std::string                      s("world");
    expt::expected<std::string, int> e(s);
    CHECK(e.has_value());
    CHECK(*e == "world");
}

// =============================================================================
// Construction from unexpected
// =============================================================================

TEST_CASE("expected: construct from unexpected<int>&&", "[ExpectedTest]") {
    expt::expected<std::string, int> e(expt::unexpected<int>(42));
    CHECK_FALSE(e.has_value());
    CHECK_FALSE(static_cast<bool>(e));
    CHECK(e.error() == 42);
}

TEST_CASE("expected: construct from const unexpected<int>&", "[ExpectedTest]") {
    const expt::unexpected<int>      u(7);
    expt::expected<std::string, int> e(u);
    CHECK_FALSE(e.has_value());
    CHECK(e.error() == 7);
}

TEST_CASE("expected: construct from unexpected<string>", "[ExpectedTest]") {
    expt::expected<int, std::string> e(expt::unexpected<std::string>("err"));
    CHECK_FALSE(e.has_value());
    CHECK(e.error() == "err");
}

// =============================================================================
// In-place construction
// =============================================================================

TEST_CASE("expected: in-place value construction int", "[ExpectedTest]") {
    expt::expected<int, std::string> e(std::in_place, 99);
    CHECK(e.has_value());
    CHECK(*e == 99);
}

TEST_CASE("expected: in-place value construction string multi-arg", "[ExpectedTest]") {
    expt::expected<std::string, int> e(std::in_place, 3u, 'x');
    CHECK(e.has_value());
    CHECK(*e == "xxx");
}

TEST_CASE("expected: in-place value construction with initializer_list", "[ExpectedTest]") {
    expt::expected<std::vector<int>, std::string> e(std::in_place, std::initializer_list<int>{1, 2, 3});
    CHECK(e.has_value());
    REQUIRE(e->size() == 3u);
    CHECK((*e)[0] == 1);
    CHECK((*e)[2] == 3);
}

TEST_CASE("expected: in-place error construction", "[ExpectedTest]") {
    expt::expected<int, std::string> e(expt::unexpect, "error message");
    CHECK_FALSE(e.has_value());
    CHECK(e.error() == "error message");
}

TEST_CASE("expected: in-place error construction with initializer_list", "[ExpectedTest]") {
    expt::expected<int, std::vector<int>> e(expt::unexpect, std::initializer_list<int>{10, 20, 30});
    CHECK_FALSE(e.has_value());
    REQUIRE(e.error().size() == 3u);
    CHECK(e.error()[1] == 20);
}

// =============================================================================
// Copy and move construction
// =============================================================================

TEST_CASE("expected: copy construct with value", "[ExpectedTest]") {
    expt::expected<int, std::string> a(42);
    expt::expected<int, std::string> b(a);
    CHECK(b.has_value());
    CHECK(*b == 42);
}

TEST_CASE("expected: copy construct with error", "[ExpectedTest]") {
    expt::expected<int, std::string> a(expt::unexpected<std::string>("fail"));
    expt::expected<int, std::string> b(a);
    CHECK_FALSE(b.has_value());
    CHECK(b.error() == "fail");
}

TEST_CASE("expected: move construct with value", "[ExpectedTest]") {
    expt::expected<std::string, int> a("moved");
    expt::expected<std::string, int> b(std::move(a));
    CHECK(b.has_value());
    CHECK(*b == "moved");
}

TEST_CASE("expected: move construct with error", "[ExpectedTest]") {
    expt::expected<int, std::string> a(expt::unexpected<std::string>("gone"));
    expt::expected<int, std::string> b(std::move(a));
    CHECK_FALSE(b.has_value());
    CHECK(b.error() == "gone");
}

// =============================================================================
// Converting construction from expected<U, G>
// =============================================================================

TEST_CASE("expected: converting copy construct value from int to long", "[ExpectedTest]") {
    expt::expected<int, int>   src(42);
    expt::expected<long, long> dst(src);
    CHECK(dst.has_value());
    CHECK(*dst == 42L);
}

TEST_CASE("expected: converting copy construct error from int to long", "[ExpectedTest]") {
    expt::expected<int, int>   src(expt::unexpected<int>(7));
    expt::expected<long, long> dst(src);
    CHECK_FALSE(dst.has_value());
    CHECK(dst.error() == 7L);
}

TEST_CASE("expected: converting move construct value from int to long", "[ExpectedTest]") {
    expt::expected<int, int>   src(99);
    expt::expected<long, long> dst(std::move(src));
    CHECK(dst.has_value());
    CHECK(*dst == 99L);
}

// =============================================================================
// Copy and move assignment
// =============================================================================

TEST_CASE("expected: copy assign value to value", "[ExpectedTest]") {
    expt::expected<int, std::string> a(1);
    expt::expected<int, std::string> b(2);
    a = b;
    CHECK(a.has_value());
    CHECK(*a == 2);
}

TEST_CASE("expected: copy assign error to error", "[ExpectedTest]") {
    expt::expected<int, std::string> a(expt::unexpected<std::string>("a"));
    expt::expected<int, std::string> b(expt::unexpected<std::string>("b"));
    a = b;
    CHECK_FALSE(a.has_value());
    CHECK(a.error() == "b");
}

TEST_CASE("expected: copy assign error to value (state change)", "[ExpectedTest]") {
    expt::expected<int, std::string> a(42);
    expt::expected<int, std::string> b(expt::unexpected<std::string>("error"));
    a = b;
    CHECK_FALSE(a.has_value());
    CHECK(a.error() == "error");
}

TEST_CASE("expected: copy assign value to error (state change)", "[ExpectedTest]") {
    expt::expected<int, std::string> a(expt::unexpected<std::string>("error"));
    expt::expected<int, std::string> b(42);
    a = b;
    CHECK(a.has_value());
    CHECK(*a == 42);
}

TEST_CASE("expected: move assign value to value", "[ExpectedTest]") {
    expt::expected<std::string, int> a("old");
    expt::expected<std::string, int> b("new");
    a = std::move(b);
    CHECK(a.has_value());
    CHECK(*a == "new");
}

TEST_CASE("expected: move assign error to value (state change)", "[ExpectedTest]") {
    expt::expected<std::string, int> a("val");
    expt::expected<std::string, int> b(expt::unexpected<int>(99));
    a = std::move(b);
    CHECK_FALSE(a.has_value());
    CHECK(a.error() == 99);
}

// =============================================================================
// Assignment from value and unexpected
// =============================================================================

TEST_CASE("expected: assign int value when has value", "[ExpectedTest]") {
    expt::expected<int, std::string> e(1);
    e = 42;
    CHECK(e.has_value());
    CHECK(*e == 42);
}

TEST_CASE("expected: assign int value when has error (state change)", "[ExpectedTest]") {
    expt::expected<int, std::string> e(expt::unexpected<std::string>("err"));
    e = 100;
    CHECK(e.has_value());
    CHECK(*e == 100);
}

TEST_CASE("expected: assign from unexpected const& when has error", "[ExpectedTest]") {
    expt::expected<int, std::string>    e(expt::unexpected<std::string>("old"));
    const expt::unexpected<std::string> u("new");
    e = u;
    CHECK_FALSE(e.has_value());
    CHECK(e.error() == "new");
}

TEST_CASE("expected: assign from unexpected const& when has value (state change)", "[ExpectedTest]") {
    expt::expected<int, std::string> e(10);
    e = expt::unexpected<std::string>("error");
    CHECK_FALSE(e.has_value());
    CHECK(e.error() == "error");
}

TEST_CASE("expected: assign from unexpected&& when has value (state change)", "[ExpectedTest]") {
    expt::expected<int, std::string> e(42);
    e = expt::unexpected<std::string>("fail");
    CHECK_FALSE(e.has_value());
    CHECK(e.error() == "fail");
}

// =============================================================================
// emplace
// =============================================================================

TEST_CASE("expected: emplace from value state", "[ExpectedTest]") {
    expt::expected<int, std::string> e(1);
    int&                             r = e.emplace(99);
    CHECK(e.has_value());
    CHECK(*e == 99);
    CHECK(r == 99);
}

TEST_CASE("expected: emplace from error state (state change)", "[ExpectedTest]") {
    expt::expected<int, std::string> e(expt::unexpected<std::string>("err"));
    int&                             r = e.emplace(42);
    CHECK(e.has_value());
    CHECK(*e == 42);
    CHECK(r == 42);
}

TEST_CASE("expected: emplace with multiple args (int)", "[ExpectedTest]") {
    // emplace requires is_nothrow_constructible; use int which is always nothrow
    expt::expected<int, std::string> e(expt::unexpected<std::string>("err"));
    int&                             r = e.emplace(77);
    CHECK(e.has_value());
    CHECK(*e == 77);
    CHECK(r == 77);
}

// =============================================================================
// swap
// =============================================================================

TEST_CASE("expected: swap value-value", "[ExpectedTest]") {
    expt::expected<int, std::string> a(1);
    expt::expected<int, std::string> b(2);
    a.swap(b);
    CHECK(*a == 2);
    CHECK(*b == 1);
}

TEST_CASE("expected: swap error-error", "[ExpectedTest]") {
    expt::expected<int, std::string> a(expt::unexpected<std::string>("a"));
    expt::expected<int, std::string> b(expt::unexpected<std::string>("b"));
    a.swap(b);
    CHECK(a.error() == "b");
    CHECK(b.error() == "a");
}

TEST_CASE("expected: swap value-error", "[ExpectedTest]") {
    expt::expected<int, std::string> a(42);
    expt::expected<int, std::string> b(expt::unexpected<std::string>("err"));
    a.swap(b);
    CHECK_FALSE(a.has_value());
    CHECK(a.error() == "err");
    CHECK(b.has_value());
    CHECK(*b == 42);
}

TEST_CASE("expected: swap error-value", "[ExpectedTest]") {
    expt::expected<int, std::string> a(expt::unexpected<std::string>("err"));
    expt::expected<int, std::string> b(42);
    a.swap(b);
    CHECK(a.has_value());
    CHECK(*a == 42);
    CHECK_FALSE(b.has_value());
    CHECK(b.error() == "err");
}

TEST_CASE("expected: friend swap function", "[ExpectedTest]") {
    expt::expected<int, std::string> a(10);
    expt::expected<int, std::string> b(20);
    swap(a, b);
    CHECK(*a == 20);
    CHECK(*b == 10);
}

// =============================================================================
// Observers: operator->, operator*, has_value, operator bool
// =============================================================================

TEST_CASE("expected: operator-> non-const", "[ExpectedTest]") {
    expt::expected<std::string, int> e("hello");
    CHECK(e->size() == 5u);
}

TEST_CASE("expected: operator-> const", "[ExpectedTest]") {
    const expt::expected<std::string, int> e("hi");
    CHECK(e->size() == 2u);
}

TEST_CASE("expected: operator* lvalue ref is mutable", "[ExpectedTest]") {
    expt::expected<int, std::string> e(7);
    int&                             r = *e;
    r                                  = 99;
    CHECK(*e == 99);
}

TEST_CASE("expected: operator* const lvalue ref", "[ExpectedTest]") {
    const expt::expected<int, std::string> e(42);
    const int&                             r = *e;
    CHECK(r == 42);
}

TEST_CASE("expected: operator* rvalue ref", "[ExpectedTest]") {
    expt::expected<std::string, int> e("move");
    std::string                      s = *std::move(e);
    CHECK(s == "move");
}

TEST_CASE("expected: operator* const rvalue ref", "[ExpectedTest]") {
    const expt::expected<std::string, int> e("cmove");
    std::string                            s = *std::move(e);
    CHECK(s == "cmove");
}

TEST_CASE("expected: has_value and operator bool", "[ExpectedTest]") {
    expt::expected<int, std::string> v(1);
    expt::expected<int, std::string> e(expt::unexpected<std::string>("e"));
    CHECK(v.has_value());
    CHECK(static_cast<bool>(v));
    CHECK_FALSE(e.has_value());
    CHECK_FALSE(static_cast<bool>(e));
}

// =============================================================================
// value() observers and throws
// =============================================================================

TEST_CASE("expected: value() lvalue ref is mutable", "[ExpectedTest]") {
    expt::expected<int, std::string> e(10);
    int&                             r = e.value();
    r                                  = 99;
    CHECK(*e == 99);
}

TEST_CASE("expected: value() const lvalue ref", "[ExpectedTest]") {
    const expt::expected<int, std::string> e(5);
    const int&                             r = e.value();
    CHECK(r == 5);
}

TEST_CASE("expected: value() rvalue ref", "[ExpectedTest]") {
    expt::expected<std::string, int> e("rval");
    std::string                      s = std::move(e).value();
    CHECK(s == "rval");
}

TEST_CASE("expected: value() throws bad_expected_access from lvalue", "[ExpectedTest]") {
    expt::expected<int, std::string> e(expt::unexpected<std::string>("bad"));
    CHECK_THROWS_AS(e.value(), expt::bad_expected_access<std::string>);
}

TEST_CASE("expected: value() throws bad_expected_access from const lvalue", "[ExpectedTest]") {
    const expt::expected<int, std::string> e(expt::unexpected<std::string>("bad"));
    CHECK_THROWS_AS(e.value(), expt::bad_expected_access<std::string>);
}

TEST_CASE("expected: value() throw carries the error value", "[ExpectedTest]") {
    expt::expected<int, std::string> e(expt::unexpected<std::string>("oops"));
    try {
        e.value();
        FAIL("should have thrown");
    } catch (const expt::bad_expected_access<std::string>& ex) {
        CHECK(ex.error() == "oops");
    }
}

TEST_CASE("expected: value() rvalue throws bad_expected_access", "[ExpectedTest]") {
    expt::expected<int, std::string> e(expt::unexpected<std::string>("bad"));
    CHECK_THROWS_AS(std::move(e).value(), expt::bad_expected_access<std::string>);
}

TEST_CASE("expected: value() throw catchable as std::exception", "[ExpectedTest]") {
    expt::expected<int, int> e(expt::unexpected<int>(99));
    try {
        e.value();
        FAIL("should have thrown");
    } catch (const std::exception& ex) {
        CHECK(std::string_view(ex.what()) == "bad expected access");
    }
}

TEST_CASE("expected: value() throw catchable as bad_expected_access<void>", "[ExpectedTest]") {
    expt::expected<int, std::string> e(expt::unexpected<std::string>("e"));
    try {
        e.value();
        FAIL("should have thrown");
    } catch (const expt::bad_expected_access<void>& ex) {
        CHECK(std::string_view(ex.what()) == "bad expected access");
    }
}

// =============================================================================
// error() observers
// =============================================================================

TEST_CASE("expected: error() lvalue ref is mutable", "[ExpectedTest]") {
    expt::expected<int, std::string> e(expt::unexpected<std::string>("err"));
    std::string&                     r = e.error();
    r                                  = "changed";
    CHECK(e.error() == "changed");
}

TEST_CASE("expected: error() const lvalue ref", "[ExpectedTest]") {
    const expt::expected<int, std::string> e(expt::unexpected<std::string>("err"));
    const std::string&                     r = e.error();
    CHECK(r == "err");
}

TEST_CASE("expected: error() rvalue ref", "[ExpectedTest]") {
    expt::expected<int, std::string> e(expt::unexpected<std::string>("rval"));
    std::string                      s = std::move(e).error();
    CHECK(s == "rval");
}

TEST_CASE("expected: error() const rvalue ref", "[ExpectedTest]") {
    const expt::expected<int, std::string> e(expt::unexpected<std::string>("crval"));
    std::string                            s = std::move(e).error();
    CHECK(s == "crval");
}

// =============================================================================
// value_or and error_or
// =============================================================================

TEST_CASE("expected: value_or returns value when has value", "[ExpectedTest]") {
    expt::expected<int, std::string> e(42);
    CHECK(e.value_or(0) == 42);
}

TEST_CASE("expected: value_or returns default when has error", "[ExpectedTest]") {
    expt::expected<int, std::string> e(expt::unexpected<std::string>("err"));
    CHECK(e.value_or(-1) == -1);
}

TEST_CASE("expected: value_or const& overload returns default for error", "[ExpectedTest]") {
    const expt::expected<int, std::string> e(expt::unexpected<std::string>("err"));
    CHECK(e.value_or(99) == 99);
}

TEST_CASE("expected: value_or && moves value out", "[ExpectedTest]") {
    expt::expected<std::string, int> e("val");
    std::string                      s = std::move(e).value_or("def");
    CHECK(s == "val");
}

TEST_CASE("expected: value_or && uses default when no value", "[ExpectedTest]") {
    expt::expected<std::string, int> e(expt::unexpected<int>(0));
    std::string                      s = std::move(e).value_or("default");
    CHECK(s == "default");
}

TEST_CASE("expected: error_or returns error when has error", "[ExpectedTest]") {
    expt::expected<int, std::string> e(expt::unexpected<std::string>("err"));
    CHECK(e.error_or("def") == "err");
}

TEST_CASE("expected: error_or returns default when has value", "[ExpectedTest]") {
    expt::expected<int, std::string> e(42);
    CHECK(e.error_or("default") == "default");
}

TEST_CASE("expected: error_or const& uses default for value", "[ExpectedTest]") {
    const expt::expected<int, std::string> e(42);
    CHECK(e.error_or("fallback") == "fallback");
}

TEST_CASE("expected: error_or && moves error out", "[ExpectedTest]") {
    expt::expected<int, std::string> e(expt::unexpected<std::string>("err"));
    std::string                      s = std::move(e).error_or("def");
    CHECK(s == "err");
}

// =============================================================================
// Equality operators
// =============================================================================

TEST_CASE("expected: equality same value", "[ExpectedTest]") {
    expt::expected<int, std::string> a(42);
    expt::expected<int, std::string> b(42);
    CHECK(a == b);
}

TEST_CASE("expected: inequality different values", "[ExpectedTest]") {
    expt::expected<int, std::string> a(1);
    expt::expected<int, std::string> b(2);
    CHECK_FALSE(a == b);
}

TEST_CASE("expected: inequality value vs error", "[ExpectedTest]") {
    expt::expected<int, std::string> a(1);
    expt::expected<int, std::string> b(expt::unexpected<std::string>("e"));
    CHECK_FALSE(a == b);
}

TEST_CASE("expected: equality same error", "[ExpectedTest]") {
    expt::expected<int, std::string> a(expt::unexpected<std::string>("err"));
    expt::expected<int, std::string> b(expt::unexpected<std::string>("err"));
    CHECK(a == b);
}

TEST_CASE("expected: inequality different errors", "[ExpectedTest]") {
    expt::expected<int, std::string> a(expt::unexpected<std::string>("a"));
    expt::expected<int, std::string> b(expt::unexpected<std::string>("b"));
    CHECK_FALSE(a == b);
}

TEST_CASE("expected: equality with value T2", "[ExpectedTest]") {
    expt::expected<int, std::string> e(42);
    CHECK(e == 42);
    CHECK_FALSE(e == 0);
}

TEST_CASE("expected: error expected not equal to value", "[ExpectedTest]") {
    expt::expected<int, std::string> e(expt::unexpected<std::string>("err"));
    CHECK_FALSE(e == 42);
}

TEST_CASE("expected: equality with unexpected", "[ExpectedTest]") {
    expt::expected<int, std::string> e(expt::unexpected<std::string>("err"));
    CHECK(e == expt::unexpected<std::string>("err"));
    CHECK_FALSE(e == expt::unexpected<std::string>("other"));
}

TEST_CASE("expected: value expected not equal to unexpected", "[ExpectedTest]") {
    expt::expected<int, std::string> e(42);
    CHECK_FALSE(e == expt::unexpected<std::string>("err"));
}

TEST_CASE("expected: cross-type equality value", "[ExpectedTest]") {
    expt::expected<int, int>   a(42);
    expt::expected<long, long> b(42L);
    CHECK(a == b);
}

TEST_CASE("expected: cross-type equality error", "[ExpectedTest]") {
    expt::expected<int, int>   a(expt::unexpected<int>(7));
    expt::expected<long, long> b(expt::unexpected<long>(7L));
    CHECK(a == b);
}

// =============================================================================
// Constexpr usage
// =============================================================================

TEST_CASE("expected: constexpr default construction", "[ExpectedTest]") {
    constexpr expt::expected<int, int> e;
    static_assert(e.has_value());
    static_assert(*e == 0);
}

TEST_CASE("expected: constexpr value construction", "[ExpectedTest]") {
    constexpr expt::expected<int, int> e(42);
    static_assert(e.has_value());
    static_assert(*e == 42);
}

TEST_CASE("expected: constexpr error construction", "[ExpectedTest]") {
    constexpr expt::expected<int, int> e(expt::unexpect, 7);
    static_assert(!e.has_value());
    static_assert(e.error() == 7);
}

TEST_CASE("expected: constexpr equality", "[ExpectedTest]") {
    constexpr expt::expected<int, int> a(42);
    constexpr expt::expected<int, int> b(42);
    static_assert(a == b);
}

// =============================================================================
// Destructor: calls destructor of the active member
// =============================================================================

TEST_CASE("expected: destructor runs for value", "[ExpectedTest]") {
    int destroyed = 0;
    struct Counted {
        int* d;
        explicit Counted(int* p) : d(p) {}
        ~Counted() { ++*d; }
    };
    {
        expt::expected<Counted, int> e(std::in_place, &destroyed);
        (void)e;
    }
    CHECK(destroyed >= 1);
}

TEST_CASE("expected: destructor runs for error", "[ExpectedTest]") {
    int destroyed = 0;
    struct Counted {
        int* d;
        explicit Counted(int* p) : d(p) {}
        ~Counted() { ++*d; }
    };
    {
        expt::expected<int, Counted> e(expt::unexpect, &destroyed);
        (void)e;
    }
    CHECK(destroyed >= 1);
}

// =============================================================================
// emplace with initializer_list overload
// =============================================================================

namespace {
// Custom type with noexcept initializer_list constructor for emplace testing
struct IListInt {
    int sum   = 0;
    int count = 0;
    IListInt(std::initializer_list<int> il) noexcept : count(static_cast<int>(il.size())) {
        for (int v : il)
            sum += v;
    }
};
} // namespace

TEST_CASE("expected: emplace with initializer_list", "[ExpectedTest]") {
    expt::expected<IListInt, int> e(expt::unexpect, 0);
    auto&                         ref = e.emplace(std::initializer_list<int>{1, 2, 3});
    REQUIRE(e.has_value());
    CHECK(e->count == 3);
    CHECK(e->sum == 6);
    CHECK(&ref == &*e);
}

// =============================================================================
// operator-> address equality
// =============================================================================

TEST_CASE("expected: operator-> returns address of value", "[ExpectedTest]") {
    expt::expected<std::string, int> e("hello");
    CHECK(e.operator->() == std::addressof(*e));
}

// =============================================================================
// Emplace constraint: nothrow_constructible_v<T, Args...> required
// (negative compile tested via expected_emplace_throwing_fail.cpp)
// =============================================================================

TEST_CASE("expected: emplace with nothrow-constructible type", "[ExpectedTest]") {
    // int is nothrow constructible — emplace must be available
    static_assert(std::is_nothrow_constructible_v<int, int>);
    expt::expected<int, std::string> e(expt::unexpect, "err");
    int&                             r = e.emplace(99);
    CHECK(r == 99);
    CHECK(e.has_value());
}

// =============================================================================
// value() mandate: Mandates is_copy_constructible_v<E>
// (negative compile tested via expected_value_mandate_fail.cpp when implemented)
// =============================================================================

TEST_CASE("expected: value() ref-qualification return types", "[ExpectedTest]") {
    static_assert(std::is_same_v<decltype(std::declval<expt::expected<int, int>&>().value()), int&>);
    static_assert(std::is_same_v<decltype(std::declval<const expt::expected<int, int>&>().value()), const int&>);
    static_assert(std::is_same_v<decltype(std::declval<expt::expected<int, int>&&>().value()), int&&>);
    static_assert(std::is_same_v<decltype(std::declval<const expt::expected<int, int>&&>().value()), const int&&>);
}
