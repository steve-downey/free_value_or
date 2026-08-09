// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include <beman/expected/expected.hpp>
#include <beman/expected/expected.hpp>

#include <catch2/catch_test_macros.hpp>

#include "testing/types.hpp"

#include <string>
#include <type_traits>

using namespace beman::expected;

// ---------------------------------------------------------------------------
// Type-level assertions
// ---------------------------------------------------------------------------

static_assert(std::is_default_constructible_v<expected<void, int&>>);
static_assert(std::is_nothrow_default_constructible_v<expected<void, int&>>);

static_assert(std::is_trivially_copy_constructible_v<expected<void, int&>>);
static_assert(std::is_trivially_move_constructible_v<expected<void, int&>>);
static_assert(std::is_trivially_destructible_v<expected<void, int&>>);
static_assert(std::is_trivially_copyable_v<expected<void, int&>>);

static_assert(std::is_same_v<decltype(std::declval<expected<void, int&>>().error()), int&>);
static_assert(std::is_same_v<decltype(std::declval<const expected<void, int&>>().error()), int&>);

static_assert(std::is_void_v<decltype(*std::declval<expected<void, int&>>())>);

// absence of operator-> and value_or tested by _fail.cpp negative compile tests

// Finding 1: copy/move assignment must be available for reference E, including
// const-reference E, where E itself is not assignable (is_copy_assignable_v<const
// int&> is false) but the stored unexpected<E&> rebinds via pointer assignment.
static_assert(std::is_copy_assignable_v<expected<void, int&>>);
static_assert(std::is_move_assignable_v<expected<void, int&>>);
static_assert(std::is_copy_assignable_v<expected<void, const int&>>);
static_assert(std::is_move_assignable_v<expected<void, const int&>>);

// Finding 2: the general (value-G) void converting constructors must be gated on
// !is_reference_v<E> — for reference E they are unsound: the lvalue form would bind into
// the source's owned error (dangling once the source is gone), and the rvalue form hard-errors
// by selecting a deleted unexpected<E&> constructor deep in the body instead of being excluded
// from overload resolution. is_constructible_v must report false for both, matching reality.
static_assert(!std::is_constructible_v<expected<void, const int&>, const expected<void, int>&>);
static_assert(!std::is_constructible_v<expected<void, const int&>, expected<void, int>&&>);
static_assert(!std::is_constructible_v<expected<void, int&>, const expected<void, int>&>);
static_assert(!std::is_constructible_v<expected<void, int&>, expected<void, int>&&>);

// The dedicated reference-E path (source error type is itself a reference) remains available,
// for both lvalue and rvalue sources.
static_assert(std::is_constructible_v<expected<void, int&>, const expected<void, int&>&>);
static_assert(std::is_constructible_v<expected<void, int&>, expected<void, int&>&&>);
static_assert(std::is_constructible_v<expected<void, const int&>, const expected<void, int&>&>);
static_assert(std::is_constructible_v<expected<void, const int&>, expected<void, int&>&&>);

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST_CASE("expected<void,E&>: default construct has value", "[expected_void_ref_e]") {
    expected<void, int&> e;
    REQUIRE(e.has_value());
    static_assert(std::is_nothrow_default_constructible_v<expected<void, int&>>);
}

TEST_CASE("expected<void,E&>: construct from unexpect+ref binds E&", "[expected_void_ref_e]") {
    int                  err = 42;
    expected<void, int&> e(unexpect, err);
    REQUIRE(!e.has_value());
    CHECK(&e.error() == &err);
    CHECK(e.error() == 42);
}

TEST_CASE("expected<void,E&>: in_place_t constructor", "[expected_void_ref_e]") {
    expected<void, int&> e(std::in_place);
    CHECK(e.has_value());
    static_assert(noexcept(expected<void, int&>(std::in_place)));
}

TEST_CASE("expected<void,E&>: copy construct from value state", "[expected_void_ref_e]") {
    expected<void, int&> a;
    expected<void, int&> b = a;
    CHECK(b.has_value());
}

TEST_CASE("expected<void,E&>: copy construct from error state", "[expected_void_ref_e]") {
    int                  err = 5;
    expected<void, int&> a(unexpect, err);
    expected<void, int&> b = a;
    REQUIRE(!b.has_value());
    CHECK(&b.error() == &err);
}

TEST_CASE("expected<void,E&>: move construct copies pointer", "[expected_void_ref_e]") {
    int                  err = 3;
    expected<void, int&> a(unexpect, err);
    expected<void, int&> b = std::move(a);
    REQUIRE(!b.has_value());
    CHECK(&b.error() == &err);
}

TEST_CASE("expected<void,E&>: convert from expected<void, G&>", "[expected_void_ref_e]") {
    int                  err = 7;
    expected<void, int&> src(unexpect, err);
    expected<void, int&> dst(src);
    REQUIRE(!dst.has_value());
    CHECK(&dst.error() == &err);
}

// Finding 2: the rvalue overload of the reference-E converting constructor was missing;
// only the const& form existed. Verify the && form works and still binds the external
// referent (never dangles — G is itself a reference to the caller's object).
TEST_CASE("expected<void,E&>: convert from expected<void, G&>&& binds external referent", "[expected_void_ref_e]") {
    int                  err = 9;
    expected<void, int&> src(unexpect, err);
    expected<void, int&> dst(std::move(src));
    REQUIRE(!dst.has_value());
    CHECK(&dst.error() == &err);
}

TEST_CASE("expected<void,const E&>: convert from expected<void, G&> (lvalue and rvalue)", "[expected_void_ref_e]") {
    int                        err = 11;
    expected<void, int&>       src1(unexpect, err);
    expected<void, const int&> dst1(src1);
    REQUIRE(!dst1.has_value());
    CHECK(&dst1.error() == &err);

    expected<void, int&>       src2(unexpect, err);
    expected<void, const int&> dst2(std::move(src2));
    REQUIRE(!dst2.has_value());
    CHECK(&dst2.error() == &err);
}

// ---------------------------------------------------------------------------
// Error rebind semantics on assignment
// ---------------------------------------------------------------------------

TEST_CASE("expected<void,E&>: rebind error via copy assign from another expected", "[expected_void_ref_e]") {
    int                  e1 = 1, e2 = 2;
    expected<void, int&> a(unexpect, e1);
    expected<void, int&> b(unexpect, e2);
    a = b;
    REQUIRE(!a.has_value());
    CHECK(&a.error() == &e2);
    CHECK(e1 == 1); // unchanged — rebind, not assign-through
}

TEST_CASE("expected<void,E&>: rebind does NOT assign through error reference", "[expected_void_ref_e]") {
    int                  e1 = 100, e2 = 200;
    expected<void, int&> a(unexpect, e1);
    expected<void, int&> b(unexpect, e2);
    a = b;
    CHECK(e1 == 100); // e1 unchanged
    CHECK(a.error() == 200);
}

TEST_CASE("expected<void,E&>: assign to value state rebinds error", "[expected_void_ref_e]") {
    int                  err = 99;
    expected<void, int&> e;
    e = expected<void, int&>(unexpect, err);
    REQUIRE(!e.has_value());
    CHECK(&e.error() == &err);
}

TEST_CASE("expected<void,E&>: assign error state to value state", "[expected_void_ref_e]") {
    int                  err = 99;
    expected<void, int&> e(unexpect, err);
    e = expected<void, int&>();
    CHECK(e.has_value());
}

TEST_CASE("expected<void,const E&>: copy assignment rebinds, does not assign through", "[expected_void_ref_e]") {
    int                        a = 1, b = 2;
    expected<void, const int&> e(unexpect, a);
    expected<void, const int&> f(unexpect, b);
    e = f;
    REQUIRE(!e.has_value());
    CHECK(&e.error() == &b);
    CHECK(a == 1);
}

TEST_CASE("expected<void,const E&>: move assignment rebinds, does not assign through", "[expected_void_ref_e]") {
    int                        a = 1, b = 2;
    expected<void, const int&> e(unexpect, a);
    expected<void, const int&> f(unexpect, b);
    e = std::move(f);
    REQUIRE(!e.has_value());
    CHECK(&e.error() == &b);
    CHECK(a == 1);
}

// ---------------------------------------------------------------------------
// Shallow const on error
// ---------------------------------------------------------------------------

TEST_CASE("expected<void,E&>: shallow const allows mutation of error referent", "[expected_void_ref_e]") {
    int                        err = 10;
    const expected<void, int&> e(unexpect, err);
    e.error() = 20;
    CHECK(err == 20);
}

// ---------------------------------------------------------------------------
// emplace() — transition to void state
// ---------------------------------------------------------------------------

TEST_CASE("expected<void,E&>: emplace from error state sets has_value", "[expected_void_ref_e]") {
    int                  err = 5;
    expected<void, int&> e(unexpect, err);
    e.emplace();
    CHECK(e.has_value());
    static_assert(noexcept(e.emplace()));
}

TEST_CASE("expected<void,E&>: emplace from value state is no-op", "[expected_void_ref_e]") {
    expected<void, int&> e;
    e.emplace();
    CHECK(e.has_value());
}

// ---------------------------------------------------------------------------
// Observers
// ---------------------------------------------------------------------------

TEST_CASE("expected<void,E&>: operator bool and has_value", "[expected_void_ref_e]") {
    expected<void, int&> a;
    int                  err = 0;
    expected<void, int&> b(unexpect, err);
    CHECK(a.has_value());
    CHECK(bool(a));
    CHECK(!b.has_value());
    CHECK(!bool(b));
}

TEST_CASE("expected<void,E&>: operator*() is void no-op", "[expected_void_ref_e]") {
    expected<void, int&> e;
    static_assert(std::is_void_v<decltype(*e)>);
    *e;
}

TEST_CASE("expected<void,E&>: value() on success is no-op", "[expected_void_ref_e]") {
    expected<void, int&> e;
    e.value();
}

TEST_CASE("expected<void,E&>: value() throws bad_expected_access on error", "[expected_void_ref_e]") {
    int                  err = 7;
    expected<void, int&> e(unexpect, err);
    REQUIRE_THROWS_AS(e.value(), beman::expected::bad_expected_access<int>);
}

TEST_CASE("expected<void,E&>: rvalue value() throws on error", "[expected_void_ref_e]") {
    int                  err = 3;
    expected<void, int&> e(unexpect, err);
    REQUIRE_THROWS_AS(std::move(e).value(), beman::expected::bad_expected_access<int>);
}

TEST_CASE("expected<void,E&>: error() returns E& with correct address", "[expected_void_ref_e]") {
    int                  err = 99;
    expected<void, int&> e(unexpect, err);
    static_assert(std::is_same_v<decltype(e.error()), int&>);
    CHECK(&e.error() == &err);
}

TEST_CASE("expected<void,E&>: error_or returns E by value", "[expected_void_ref_e]") {
    int                  err = 7;
    expected<void, int&> a(unexpect, err);
    expected<void, int&> b;
    CHECK(a.error_or(0) == 7);
    CHECK(b.error_or(42) == 42);
}

// ---------------------------------------------------------------------------
// Swap
// ---------------------------------------------------------------------------

TEST_CASE("expected<void,E&>: swap value-value (no-op)", "[expected_void_ref_e]") {
    expected<void, int&> a, b;
    a.swap(b);
    CHECK(a.has_value());
    CHECK(b.has_value());
}

TEST_CASE("expected<void,E&>: swap value-error", "[expected_void_ref_e]") {
    int                  err = 42;
    expected<void, int&> a, b(unexpect, err);
    a.swap(b);
    REQUIRE(!a.has_value());
    REQUIRE(b.has_value());
    CHECK(&a.error() == &err);
}

TEST_CASE("expected<void,E&>: swap error-value", "[expected_void_ref_e]") {
    int                  err = 5;
    expected<void, int&> a(unexpect, err), b;
    a.swap(b);
    REQUIRE(a.has_value());
    REQUIRE(!b.has_value());
    CHECK(&b.error() == &err);
}

TEST_CASE("expected<void,E&>: swap error-error rebinds pointers", "[expected_void_ref_e]") {
    int                  e1 = 1, e2 = 2;
    expected<void, int&> a(unexpect, e1), b(unexpect, e2);
    a.swap(b);
    CHECK(&a.error() == &e2);
    CHECK(&b.error() == &e1);
    CHECK(e1 == 1);
    CHECK(e2 == 2);
}

// ---------------------------------------------------------------------------
// Equality
// ---------------------------------------------------------------------------

TEST_CASE("expected<void,E&>: equality both have values", "[expected_void_ref_e]") {
    expected<void, int&> a, b;
    CHECK(a == b);
}

TEST_CASE("expected<void,E&>: equality both have errors (same value)", "[expected_void_ref_e]") {
    int                  e1 = 5, e2 = 5;
    expected<void, int&> a(unexpect, e1), b(unexpect, e2);
    CHECK(a == b);
}

TEST_CASE("expected<void,E&>: equality mixed value/error", "[expected_void_ref_e]") {
    expected<void, int&> a;
    int                  err = 0;
    expected<void, int&> b(unexpect, err);
    CHECK(!(a == b));
}

TEST_CASE("expected<void,E&>: equality with unexpected", "[expected_void_ref_e]") {
    int                  err = 7;
    expected<void, int&> e(unexpect, err);
    CHECK(e == unexpected(7));
    CHECK(!(e == unexpected(8)));
}

// ---------------------------------------------------------------------------
// Monadic operations — void value + reference error
// ---------------------------------------------------------------------------

TEST_CASE("expected<void,E&>: and_then calls F with no args", "[expected_void_ref_e]") {
    expected<void, int&> e;
    int                  calls = 0;
    auto                 r     = e.and_then([&]() -> expected<int, int&> {
        ++calls;
        return 42;
    });
    CHECK(calls == 1);
    REQUIRE(r.has_value());
    CHECK(*r == 42);
}

TEST_CASE("expected<void,E&>: and_then short-circuits on error", "[expected_void_ref_e]") {
    int                  err = 3;
    expected<void, int&> e(unexpect, err);
    bool                 called = false;
    auto                 r      = e.and_then([&]() -> expected<int, int&> {
        called = true;
        return 0;
    });
    CHECK(!called);
    REQUIRE(!r.has_value());
    CHECK(&r.error() == &err);
}

TEST_CASE("expected<void,E&>: or_else receives E& and can return success", "[expected_void_ref_e]") {
    int                  err = 5;
    expected<void, int&> e(unexpect, err);
    auto                 r = e.or_else([](int& v) -> expected<void, int&> {
        (void)v;
        return {};
    });
    CHECK(r.has_value());
}

TEST_CASE("expected<void,E&>: or_else propagates E& to F", "[expected_void_ref_e]") {
    int                  err = 5;
    expected<void, int&> e(unexpect, err);
    int*                 seen = nullptr;
    auto                 r    = e.or_else([&](int& v) -> expected<void, int&> {
        seen = &v;
        return expected<void, int&>(unexpect, v);
    });
    CHECK(seen == &err);
    REQUIRE(!r.has_value());
    CHECK(r.error() == 5);
}

TEST_CASE("expected<void,E&>: or_else short-circuits on success", "[expected_void_ref_e]") {
    expected<void, int&> e;
    bool                 called = false;
    int                  dummy  = 0;
    auto                 r      = e.or_else([&](int&) -> expected<void, int&> {
        called = true;
        return expected<void, int&>(unexpect, dummy);
    });
    CHECK(!called);
    CHECK(r.has_value());
}

TEST_CASE("expected<void,E&>: transform calls F with no args", "[expected_void_ref_e]") {
    expected<void, int&> e;
    auto                 r = e.transform([]() { return 42; });
    static_assert(std::is_same_v<decltype(r), expected<int, int&>>);
    REQUIRE(r.has_value());
    CHECK(*r == 42);
}

TEST_CASE("expected<void,E&>: transform with void-returning F", "[expected_void_ref_e]") {
    expected<void, int&> e;
    int                  count = 0;
    auto                 r     = e.transform([&]() { ++count; });
    static_assert(std::is_same_v<decltype(r), expected<void, int&>>);
    CHECK(r.has_value());
    CHECK(count == 1);
}

TEST_CASE("expected<void,E&>: transform short-circuits on error", "[expected_void_ref_e]") {
    int                  err = 9;
    expected<void, int&> e(unexpect, err);
    bool                 called = false;
    auto                 r      = e.transform([&]() -> int {
        called = true;
        return 0;
    });
    CHECK(!called);
    REQUIRE(!r.has_value());
    CHECK(&r.error() == &err);
}

TEST_CASE("expected<void,E&>: transform_error transforms E& to new type", "[expected_void_ref_e]") {
    int                  err = 3;
    expected<void, int&> e(unexpect, err);
    auto                 r = e.transform_error([](int& v) -> std::string { return std::to_string(v); });
    static_assert(std::is_same_v<decltype(r), expected<void, std::string>>);
    REQUIRE(!r.has_value());
    CHECK(r.error() == "3");
}

TEST_CASE("expected<void,E&>: transform_error with value short-circuits", "[expected_void_ref_e]") {
    expected<void, int&> e;
    bool                 called = false;
    auto                 r      = e.transform_error([&](int&) -> std::string {
        called = true;
        return "";
    });
    CHECK(!called);
    CHECK(r.has_value());
}

// ---------------------------------------------------------------------------
// End-to-end chaining
// ---------------------------------------------------------------------------

TEST_CASE("expected<void,E&>: monadic chaining happy path", "[expected_void_ref_e]") {
    auto r = expected<void, int&>{}.and_then([]() -> expected<int, int&> { return 42; }).transform([](int v) {
        return v * 2;
    });
    REQUIRE(r.has_value());
    CHECK(*r == 84);
}

TEST_CASE("expected<void,E&>: monadic chaining error path", "[expected_void_ref_e]") {
    int  err = 0;
    auto r   = expected<void, int&>(unexpect, err)
                   .and_then([]() -> expected<int, int&> { return 0; })
                   .transform_error([](int& v) -> std::string { return std::to_string(v); });
    REQUIRE(!r.has_value());
}

// ---------------------------------------------------------------------------
// Triviality
// ---------------------------------------------------------------------------

TEST_CASE("expected<void,E&>: trivial operations", "[expected_void_ref_e]") {
    static_assert(std::is_trivially_copyable_v<expected<void, int&>>);
    static_assert(std::is_trivially_destructible_v<expected<void, int&>>);
}

// =============================================================================
// Additional coverage tests: void,E& rvalue/const-rvalue monadic paths
// =============================================================================
using namespace beman::expected::testing;

// --- Monadic: and_then rvalue/const-rvalue on ERROR state ---
TEST_CASE("and_then rvalue on error short-circuits", "[expected_void_ref_e]") {
    int                  err = 5;
    expected<void, int&> e(unexpect, err);
    auto                 r = std::move(e).and_then([]() -> expected<void, int&> { return {}; });
    REQUIRE(!r.has_value());
}

TEST_CASE("and_then const rvalue on error short-circuits", "[expected_void_ref_e]") {
    int                        err = 5;
    const expected<void, int&> e(unexpect, err);
    auto                       r = std::move(e).and_then([]() -> expected<void, int&> { return {}; });
    REQUIRE(!r.has_value());
}

// --- Monadic: and_then rvalue/const-rvalue on VALUE state ---
TEST_CASE("and_then rvalue on value calls F", "[expected_void_ref_e]") {
    expected<void, int&> e;
    bool                 called = false;
    auto                 r      = std::move(e).and_then([&]() -> expected<int, int&> {
        called = true;
        return 42;
    });
    CHECK(called);
    REQUIRE(r.has_value());
}

TEST_CASE("and_then const rvalue on value calls F", "[expected_void_ref_e]") {
    const expected<void, int&> e;
    auto                       r = std::move(e).and_then([]() -> expected<int, int&> { return 99; });
    REQUIRE(r.has_value());
    CHECK(*r == 99);
}

// --- Monadic: or_else rvalue/const-rvalue on VALUE state ---
TEST_CASE("or_else rvalue on value short-circuits", "[expected_void_ref_e]") {
    expected<void, int&> e;
    auto                 r = std::move(e).or_else([](int&) -> expected<void, int&> { return {}; });
    REQUIRE(r.has_value());
}

TEST_CASE("or_else const rvalue on value short-circuits", "[expected_void_ref_e]") {
    const expected<void, int&> e;
    auto                       r = std::move(e).or_else([](int&) -> expected<void, int&> { return {}; });
    REQUIRE(r.has_value());
}

// --- Monadic: transform rvalue/const-rvalue ---
TEST_CASE("transform rvalue on error short-circuits", "[expected_void_ref_e]") {
    int                  err = 3;
    expected<void, int&> e(unexpect, err);
    auto                 r = std::move(e).transform([]() { return 1; });
    REQUIRE(!r.has_value());
}

TEST_CASE("transform const rvalue on error short-circuits", "[expected_void_ref_e]") {
    int                        err = 3;
    const expected<void, int&> e(unexpect, err);
    auto                       r = std::move(e).transform([]() { return 1; });
    REQUIRE(!r.has_value());
}

TEST_CASE("transform rvalue on value calls F", "[expected_void_ref_e]") {
    expected<void, int&> e;
    auto                 r = std::move(e).transform([]() { return 42; });
    REQUIRE(r.has_value());
    CHECK(*r == 42);
}

TEST_CASE("transform const rvalue on value calls F", "[expected_void_ref_e]") {
    const expected<void, int&> e;
    auto                       r = std::move(e).transform([]() { return 7; });
    REQUIRE(r.has_value());
    CHECK(*r == 7);
}

// --- Monadic: transform_error rvalue/const-rvalue ---
TEST_CASE("transform_error rvalue on value short-circuits", "[expected_void_ref_e]") {
    expected<void, int&> e;
    auto                 r = std::move(e).transform_error([](int& v) { return v * 2; });
    REQUIRE(r.has_value());
}

TEST_CASE("transform_error const rvalue on value short-circuits", "[expected_void_ref_e]") {
    const expected<void, int&> e;
    auto                       r = std::move(e).transform_error([](int& v) { return v * 2; });
    REQUIRE(r.has_value());
}

TEST_CASE("transform_error rvalue on error calls F", "[expected_void_ref_e]") {
    int                  err = 5;
    expected<void, int&> e(unexpect, err);
    auto                 r = std::move(e).transform_error([](int& v) { return v + 1; });
    REQUIRE(!r.has_value());
    CHECK(r.error() == 6);
}

TEST_CASE("transform_error const rvalue on error calls F", "[expected_void_ref_e]") {
    int                        err = 5;
    const expected<void, int&> e(unexpect, err);
    auto                       r = std::move(e).transform_error([](int& v) { return v + 1; });
    REQUIRE(!r.has_value());
    CHECK(r.error() == 6);
}

// --- value() throw paths ---
TEST_CASE("value() throws on error state", "[expected_void_ref_e]") {
    int                  err = 42;
    expected<void, int&> e(unexpect, err);
    CHECK_THROWS_AS(e.value(), beman::expected::bad_expected_access<int>);
}

TEST_CASE("value() rvalue throws on error state", "[expected_void_ref_e]") {
    int                  err = 42;
    expected<void, int&> e(unexpect, err);
    CHECK_THROWS_AS(std::move(e).value(), beman::expected::bad_expected_access<int>);
}

// --- Cross-type equality ---
TEST_CASE("equality error vs value", "[expected_void_ref_e]") {
    int                  err = 0;
    expected<void, int&> a;
    expected<void, int&> b(unexpect, err);
    CHECK_FALSE(a == b);
}

// ---------------------------------------------------------------------------
// expected<void, E&>: lvalue monadic paths
// ---------------------------------------------------------------------------

TEST_CASE("ref<void,E&>: and_then lvalue error path", "[expected_void_ref_e][traced]") {
    int                  err = 5;
    expected<void, int&> e(unexpect, err);
    auto                 r = e.and_then([]() -> expected<void, int&> { return {}; });
    REQUIRE(!r.has_value());
}

TEST_CASE("ref<void,E&>: and_then const lvalue error path", "[expected_void_ref_e][traced]") {
    int                        err = 5;
    const expected<void, int&> e(unexpect, err);
    auto                       r = e.and_then([]() -> expected<void, int&> { return {}; });
    REQUIRE(!r.has_value());
}

TEST_CASE("ref<void,E&>: or_else lvalue value path", "[expected_void_ref_e][traced]") {
    expected<void, int&> e;
    auto                 r = e.or_else([](int&) -> expected<void, int&> { return {}; });
    REQUIRE(r.has_value());
}

TEST_CASE("ref<void,E&>: or_else rvalue value path", "[expected_void_ref_e][traced]") {
    expected<void, int&> e;
    auto                 r = std::move(e).or_else([](int&) -> expected<void, int&> { return {}; });
    REQUIRE(r.has_value());
}

TEST_CASE("ref<void,E&>: or_else const rvalue value path", "[expected_void_ref_e][traced]") {
    const expected<void, int&> e;
    auto                       r = std::move(e).or_else([](int&) -> expected<void, int&> { return {}; });
    REQUIRE(r.has_value());
}

TEST_CASE("ref<void,E&>: transform lvalue value path", "[expected_void_ref_e][traced]") {
    expected<void, int&> e;
    auto                 r = e.transform([]() { return 42; });
    REQUIRE(r.has_value());
    CHECK(*r == 42);
}

TEST_CASE("ref<void,E&>: transform lvalue error path", "[expected_void_ref_e][traced]") {
    int                  err = 3;
    expected<void, int&> e(unexpect, err);
    auto                 r = e.transform([]() { return 0; });
    REQUIRE(!r.has_value());
}

TEST_CASE("ref<void,E&>: transform rvalue value path", "[expected_void_ref_e][traced]") {
    expected<void, int&> e;
    auto                 r = std::move(e).transform([]() { return 42; });
    REQUIRE(r.has_value());
}

TEST_CASE("ref<void,E&>: transform const rvalue value path", "[expected_void_ref_e][traced]") {
    const expected<void, int&> e;
    auto                       r = std::move(e).transform([]() { return 42; });
    REQUIRE(r.has_value());
}

TEST_CASE("ref<void,E&>: transform to void lvalue", "[expected_void_ref_e][traced]") {
    expected<void, int&> e;
    auto                 r = e.transform([]() {});
    REQUIRE(r.has_value());
}

TEST_CASE("ref<void,E&>: transform_error lvalue value path", "[expected_void_ref_e][traced]") {
    expected<void, int&> e;
    auto                 r = e.transform_error([](int&) { return 0; });
    REQUIRE(r.has_value());
}

TEST_CASE("ref<void,E&>: transform_error rvalue value path", "[expected_void_ref_e][traced]") {
    expected<void, int&> e;
    auto                 r = std::move(e).transform_error([](int&) { return 0; });
    REQUIRE(r.has_value());
}

TEST_CASE("ref<void,E&>: transform_error const rvalue value path", "[expected_void_ref_e][traced]") {
    const expected<void, int&> e;
    auto                       r = std::move(e).transform_error([](int&) { return 0; });
    REQUIRE(r.has_value());
}

TEST_CASE("ref<void,E&>: value() lvalue throws", "[expected_void_ref_e][traced]") {
    int                  err = 42;
    expected<void, int&> e(unexpect, err);
    CHECK_THROWS_AS(e.value(), beman::expected::bad_expected_access<int>);
}
