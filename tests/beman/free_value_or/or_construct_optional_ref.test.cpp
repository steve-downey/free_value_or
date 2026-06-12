// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// or_construct with optional<T&> (C++26 reference-optional via beman::optional).
// Feature-gated by FVO_HAS_OPTIONAL_REF (injected by fvo_add_test; always 1 in CI).
//
// Key type-theory note:
//   optional<int&>::operator*() const -> int&
//   iter_reference_t<optional<int&>> = int&
//   R (default Ret) = remove_cvref_t<int&> = int      <- always a VALUE, not a reference
//
// Contrast with reference_or: that returns a reference (int&).
// or_construct always returns a prvalue of the decayed type; it can never dangle.

#include <beman/free_value_or/value_or.hpp>

#include "test_types.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <type_traits>

#if FVO_HAS_OPTIONAL_REF

// ==========================================================================
// nullable concept: optional<int&> must satisfy nullable
// ==========================================================================
static_assert(fvo::nullable<fvo_opt::optional<int&>>,
              "optional<int&> must satisfy the nullable concept");

// ==========================================================================
// Return-type proof
//
// iter_reference_t<optional<int&>> = int&  (operator*() const returns int&)
// R (default Ret) = remove_cvref_t<int&>  = int
//
// or_construct always returns a value type — it can never produce a dangling
// reference unlike reference_or.
// ==========================================================================
static_assert(std::is_same_v<
    decltype(fvo::or_construct(std::declval<fvo_opt::optional<int&>&>(), 0)),
    int>,
    "default-Ret or_construct with optional<int&> must return int (value, not reference)");

// Explicit Ret = long: payload int& converts to long
static_assert(std::is_same_v<
    decltype(fvo::or_construct<long>(std::declval<fvo_opt::optional<int&>&>(), 0L)),
    long>,
    "explicit-Ret or_construct<long> with optional<int&> must return long");

// ==========================================================================
// Engaged: or_construct returns the referent's value
// ==========================================================================

TEST_CASE("or_construct: optional<int&> engaged returns referent's value", "[or_construct][optional_ref]") {
    int a = 7;
    fvo_opt::optional<int&> o(a);
    auto v = fvo::or_construct(o, 99);
    CHECK(v == 7);
}

TEST_CASE("or_construct: optional<int&> engaged result is a value copy (not address-of referent)", "[or_construct][optional_ref]") {
    int a = 7;
    fvo_opt::optional<int&> o(a);
    auto v = fvo::or_construct(o, 99);
    // v is a separate int, not a reference to a
    static_assert(std::is_same_v<decltype(v), int>);
    CHECK(v == 7);
    a = 42;
    CHECK(v == 7);  // v is a copy; mutation of a has no effect
}

// ==========================================================================
// Disengaged: or_construct constructs the fallback value
// ==========================================================================

TEST_CASE("or_construct: optional<int&> disengaged constructs from args", "[or_construct][optional_ref]") {
    fvo_opt::optional<int&> d;
    auto v = fvo::or_construct(d, 5);
    CHECK(v == 5);
}

TEST_CASE("or_construct: optional<int&> result is always a value, not a reference", "[or_construct][optional_ref]") {
    // Disengaged: result is int(5), a fresh value — NOT a reference to anything.
    // Contrast reference_or, which would return an int& to the fallback argument.
    fvo_opt::optional<int&> d;
    int fallback = 5;
    static_assert(std::is_same_v<decltype(fvo::or_construct(d, fallback)), int>,
                  "or_construct always returns a value type, never a reference");
    auto v = fvo::or_construct(d, fallback);
    CHECK(v == 5);
}

// ==========================================================================
// Explicit Ret — engaged and disengaged
// ==========================================================================

TEST_CASE("or_construct<long>: optional<int&> engaged returns held int as long", "[or_construct][optional_ref]") {
    int a = 7;
    fvo_opt::optional<int&> o(a);
    long v = fvo::or_construct<long>(o, 0L);
    CHECK(v == 7L);
    static_assert(std::is_same_v<decltype(fvo::or_construct<long>(o, 0L)), long>);
}

TEST_CASE("or_construct<long>: optional<int&> disengaged constructs long from args", "[or_construct][optional_ref]") {
    fvo_opt::optional<int&> d;
    long v = fvo::or_construct<long>(d, 42L);
    CHECK(v == 42L);
}

// ==========================================================================
// Multi-arg / initializer_list case with optional<string&>
//
// Disengaged optional<string&>: constructs a fresh std::string from {'a','b','c'}.
// Result is a VALUE std::string, not a reference — or_construct cannot dangle.
// ==========================================================================

TEST_CASE("or_construct: optional<string&> disengaged constructs string from init-list", "[or_construct][optional_ref]") {
    fvo_opt::optional<std::string&> d;
    // The init-list overload constructs R from {'a','b','c'}.
    // R = remove_cvref_t<iter_reference_t<optional<string&>>> = string.
    auto v = fvo::or_construct(d, {'a', 'b', 'c'});
    CHECK(v == "abc");
    static_assert(std::is_same_v<decltype(v), std::string>);
}

TEST_CASE("or_construct: optional<string&> engaged returns referent value (init-list unused)", "[or_construct][optional_ref]") {
    std::string s = "hello";
    fvo_opt::optional<std::string&> o(s);
    auto v = fvo::or_construct(o, {'x', 'y', 'z'});
    CHECK(v == "hello");
    // v is a copy; mutating s does not affect v
    s = "world";
    CHECK(v == "hello");
}

// ==========================================================================
// Value categories: lvalue optional, const optional
// ==========================================================================

TEST_CASE("or_construct: lvalue optional<int&> engaged", "[or_construct][optional_ref]") {
    int a = 10;
    fvo_opt::optional<int&> o(a);
    CHECK(fvo::or_construct(o, 0) == 10);
}

TEST_CASE("or_construct: const optional<int&> engaged", "[or_construct][optional_ref]") {
    int a = 10;
    const fvo_opt::optional<int&> o(a);
    CHECK(fvo::or_construct(o, 0) == 10);
}

TEST_CASE("or_construct: rebind optional<int&> — or_construct follows new referent", "[or_construct][optional_ref]") {
    int a = 10, b = 20;
    fvo_opt::optional<int&> o(a);
    CHECK(fvo::or_construct(o, 0) == 10);
    o = b;
    CHECK(fvo::or_construct(o, 0) == 20);
}

#endif  // FVO_HAS_OPTIONAL_REF

// Catch2 requires at least one unconditional TEST_CASE.
TEST_CASE("or_construct optional_ref tests compiled and feature gate active", "[or_construct][optional_ref]") {
#if FVO_HAS_OPTIONAL_REF
    SUCCEED("FVO_HAS_OPTIONAL_REF=1: optional<T&> or_construct tests ran above");
#else
    SUCCEED("FVO_HAS_OPTIONAL_REF=0: optional<T&> or_construct tests skipped (toolchain limitation)");
#endif
}
