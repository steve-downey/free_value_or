// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Tests for value_or, reference_or, and or_invoke with optional<T&>.
// Uses beman::optional (vendored) via the fvo_opt alias from test_types.hpp.
// The CMake target injects FVO_HAS_OPTIONAL_REF=1, so the #if gates are active.

#include <beman/free_value_or/value_or.hpp>

#include "test_types.hpp"

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

#if FVO_HAS_OPTIONAL_REF

// ==========================================================================
// nullable concept: optional<int&> must satisfy nullable
// ==========================================================================
static_assert(fvo::nullable<fvo_opt::optional<int&>>,
              "optional<int&> must satisfy the nullable concept");

// ==========================================================================
// Return-type static_asserts
//
// iter_reference_t<optional<int&>&> = int& (operator*() const returns T& = int&)
//
// value_or:     R = common_type_t<int&, int>  = int   (always a value)
// reference_or: R = common_reference_t<int&, int&> = int& (reference)
// ==========================================================================

// value_or with optional<int&> lvalue + int lvalue → int (prvalue)
static_assert(std::is_same_v<
    decltype(fvo::value_or(std::declval<fvo_opt::optional<int&>&>(), std::declval<int&>())),
    int>);

// reference_or with optional<int&> lvalue + int lvalue → int&
static_assert(std::is_same_v<
    decltype(fvo::reference_or(std::declval<fvo_opt::optional<int&>&>(), std::declval<int&>())),
    int&>);

// reference_or with optional<int&> lvalue + const int lvalue → const int&
static_assert(std::is_same_v<
    decltype(fvo::reference_or(std::declval<fvo_opt::optional<int&>&>(), std::declval<const int&>())),
    const int&>);

// ==========================================================================
// value_or: engaged returns referent's value; disengaged returns fallback
// ==========================================================================

TEST_CASE("value_or: optional<int&> engaged returns referent value", "[optional_ref][value_or]") {
    int a = 10;
    fvo_opt::optional<int&> o(a);
    int fallback = 99;
    auto v = fvo::value_or(o, fallback);
    CHECK(v == 10);
}

TEST_CASE("value_or: optional<int&> disengaged returns fallback value", "[optional_ref][value_or]") {
    fvo_opt::optional<int&> d;
    int fallback = 99;
    auto v = fvo::value_or(d, fallback);
    CHECK(v == 99);
}

TEST_CASE("value_or: optional<int&> result is a value copy, not a reference", "[optional_ref][value_or]") {
    int a = 10;
    fvo_opt::optional<int&> o(a);
    int fallback = 99;
    // R = common_type_t<int&, int&> = int, confirmed at static_assert above
    static_assert(std::is_same_v<decltype(fvo::value_or(o, fallback)), int>);
}

// ==========================================================================
// reference_or: engaged returns reference to referent (identity check)
// ==========================================================================

TEST_CASE("reference_or: optional<int&> engaged returns reference to referent", "[optional_ref][reference_or]") {
    int a = 42;
    fvo_opt::optional<int&> o(a);
    int fallback = 0;
    int& r = fvo::reference_or(o, fallback);
    CHECK(&r == &a);   // identity: same address as the referent
    CHECK(r == 42);
}

TEST_CASE("reference_or: optional<int&> disengaged returns reference to fallback", "[optional_ref][reference_or]") {
    fvo_opt::optional<int&> d;
    int fallback = 99;
    int& r = fvo::reference_or(d, fallback);
    CHECK(&r == &fallback);
    CHECK(r == 99);
}

TEST_CASE("reference_or: mutation through reference changes referent", "[optional_ref][reference_or]") {
    int a = 42;
    fvo_opt::optional<int&> o(a);
    int fallback = 0;
    int& r = fvo::reference_or(o, fallback);
    r = 99;
    CHECK(a == 99);     // referent was mutated
    CHECK(fallback == 0);  // fallback untouched
}

TEST_CASE("reference_or: mutation through reference changes fallback when disengaged", "[optional_ref][reference_or]") {
    fvo_opt::optional<int&> d;
    int fallback = 0;
    int& r = fvo::reference_or(d, fallback);
    r = 99;
    CHECK(fallback == 99);
}

// ==========================================================================
// or_invoke: engaged skips invocable; disengaged calls it exactly once
// ==========================================================================

TEST_CASE("or_invoke: optional<int&> engaged returns referent value without calling f", "[optional_ref][or_invoke]") {
    int a = 10;
    fvo_opt::optional<int&> o(a);
    int call_count = 0;
    auto v = fvo::or_invoke(o, [&] { ++call_count; return 99; });
    CHECK(v == 10);
    CHECK(call_count == 0);
}

TEST_CASE("or_invoke: optional<int&> disengaged calls f exactly once", "[optional_ref][or_invoke]") {
    fvo_opt::optional<int&> d;
    int call_count = 0;
    auto v = fvo::or_invoke(d, [&] { ++call_count; return 99; });
    CHECK(v == 99);
    CHECK(call_count == 1);
}

// ==========================================================================
// Rebinding semantics: change what the optional<int&> refers to and confirm
// the functions follow the new referent
// ==========================================================================

TEST_CASE("rebinding: value_or follows new referent after rebind", "[optional_ref][rebinding]") {
    int a = 10, b = 20;
    fvo_opt::optional<int&> o(a);
    CHECK(fvo::value_or(o, 0) == 10);

    o = b;  // rebind to b
    CHECK(fvo::value_or(o, 0) == 20);
}

TEST_CASE("rebinding: reference_or identity follows new referent after rebind", "[optional_ref][rebinding]") {
    int a = 10, b = 20;
    fvo_opt::optional<int&> o(a);
    int fallback = 0;

    int& r1 = fvo::reference_or(o, fallback);
    CHECK(&r1 == &a);

    o = b;
    int& r2 = fvo::reference_or(o, fallback);
    CHECK(&r2 == &b);
}

TEST_CASE("rebinding: disengaging after engage falls through to fallback", "[optional_ref][rebinding]") {
    int a = 10;
    fvo_opt::optional<int&> o(a);
    int fallback = 99;
    CHECK(fvo::value_or(o, fallback) == 10);

    o.reset();
    CHECK(fvo::value_or(o, fallback) == 99);
}

#endif  // FVO_HAS_OPTIONAL_REF

// Catch2 requires at least one TEST_CASE that runs unconditionally,
// even when all real tests are feature-gated.
TEST_CASE("optional_ref tests compiled and feature gate active") {
#if FVO_HAS_OPTIONAL_REF
    SUCCEED("FVO_HAS_OPTIONAL_REF=1: optional<T&> tests ran above");
#else
    SUCCEED("FVO_HAS_OPTIONAL_REF=0: optional<T&> tests skipped (toolchain limitation)");
#endif
}
