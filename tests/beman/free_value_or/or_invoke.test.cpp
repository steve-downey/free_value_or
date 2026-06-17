// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/free_value_or/value_or.hpp>
#include "test_types.hpp"

#include <catch2/catch_test_macros.hpp>

#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>

// ==========================================================================
// Return-type static_asserts
//
// R = common_type_t<iter_reference_t<T>, invoke_result_t<I>>
//
// For optional<int> lvalue: iter_reference_t = int&.
// common_type strips references, so common_type_t<int&, int> = int.
// ==========================================================================

// optional<int> + [] { return int; } → common_type<int&, int> = int
static_assert(
    std::is_same_v<decltype(fvo::or_invoke(std::declval<std::optional<int>&>(), std::declval<int (*)()>())), int>);

// optional<int> + [] { return long; } → common_type<int&, long> = long
static_assert(
    std::is_same_v<decltype(fvo::or_invoke(std::declval<std::optional<int>&>(), std::declval<long (*)()>())), long>);

// optional<int> + ref-returning invocable → invoke_result = int&,
// common_type_t<int&, int&> = int (common_type decays references).
// Contrast: reference_or uses common_reference_t which preserves references.
static_assert(
    std::is_same_v<decltype(fvo::or_invoke(std::declval<std::optional<int>&>(), std::declval<int& (*)()>())), int>);

// ==========================================================================
// Runtime: engaged → *m, disengaged → f()
// ==========================================================================

TEST_CASE("or_invoke: optional<int> engaged and disengaged", "[or_invoke]") {
    auto engaged    = NullableFixture<int>::opt_engaged(42);
    auto disengaged = NullableFixture<int>::opt_disengaged();

    CHECK(fvo::or_invoke(engaged, [] { return 0; }) == 42);
    CHECK(fvo::or_invoke(disengaged, [] { return 7; }) == 7);
}

TEST_CASE("or_invoke: expected<int,int> engaged and disengaged", "[or_invoke]") {
    auto engaged    = NullableFixture<int>::exp_engaged(42);
    auto disengaged = NullableFixture<int>::exp_disengaged();

    CHECK(fvo::or_invoke(engaged, [] { return 0; }) == 42);
    CHECK(fvo::or_invoke(disengaged, [] { return 7; }) == 7);
}

TEST_CASE("or_invoke: int* engaged and disengaged", "[or_invoke]") {
    int  obj        = 42;
    int* engaged    = raw_engaged<int>(obj);
    int* disengaged = raw_disengaged<int>();

    CHECK(fvo::or_invoke(engaged, [] { return 0; }) == 42);
    CHECK(fvo::or_invoke(disengaged, [] { return 7; }) == 7);
}

TEST_CASE("or_invoke: shared_ptr<int> engaged and disengaged", "[or_invoke]") {
    auto engaged    = NullableFixture<int>::sptr_engaged(42);
    auto disengaged = NullableFixture<int>::sptr_disengaged();

    CHECK(fvo::or_invoke(engaged, [] { return 0; }) == 42);
    CHECK(fvo::or_invoke(disengaged, [] { return 7; }) == 7);
}

TEST_CASE("or_invoke: unique_ptr<int> engaged and disengaged", "[or_invoke]") {
    auto disengaged = NullableFixture<int>::uptr_disengaged();
    CHECK(fvo::or_invoke(disengaged, [] { return 7; }) == 7);

    // unique_ptr is move-only; pass as rvalue for the engaged case
    CHECK(fvo::or_invoke(NullableFixture<int>::uptr_engaged(42), [] { return 0; }) == 42);
}

// ==========================================================================
// Laziness (headline property): invocable called ONLY when disengaged
// ==========================================================================

TEST_CASE("or_invoke: laziness — invocable not called when engaged", "[or_invoke][laziness]") {
    int  call_count = 0;
    auto f          = [&] {
        ++call_count;
        return 99;
    };

    std::optional<int> engaged{42};
    (void)fvo::or_invoke(engaged, f);
    CHECK(call_count == 0); // NOT invoked when engaged
}

TEST_CASE("or_invoke: laziness — invocable called exactly once when disengaged", "[or_invoke][laziness]") {
    int  call_count = 0;
    auto f          = [&] {
        ++call_count;
        return 99;
    };

    std::optional<int> disengaged{};
    CHECK(fvo::or_invoke(disengaged, f) == 99);
    CHECK(call_count == 1); // invoked exactly once
}

TEST_CASE("or_invoke: laziness — contrast with value_or eagerness", "[or_invoke][laziness]") {
    // value_or evaluates its fallback eagerly (argument evaluation).
    // or_invoke evaluates its invocable lazily (called only when disengaged).
    int or_invoke_count = 0;
    int value_or_count  = 0;

    std::optional<int> engaged{42};

    auto lazy_f = [&] {
        ++or_invoke_count;
        return 0;
    };

    fvo::or_invoke(engaged, lazy_f);
    fvo::value_or(engaged, (++value_or_count, 0));

    CHECK(or_invoke_count == 0); // lazy: not called
    CHECK(value_or_count == 1);  // eager: always evaluated
}

// ==========================================================================
// Value categories of m
// ==========================================================================

TEST_CASE("or_invoke: value categories of m", "[or_invoke]") {
    SECTION("lvalue engaged") {
        std::optional<int> m{42};
        int                count = 0;
        auto               f     = [&] {
            ++count;
            return 0;
        };
        CHECK(fvo::or_invoke(m, f) == 42);
        CHECK(count == 0);
    }
    SECTION("const lvalue engaged") {
        const std::optional<int> m{42};
        int                      count = 0;
        auto                     f     = [&] {
            ++count;
            return 0;
        };
        CHECK(fvo::or_invoke(m, f) == 42);
        CHECK(count == 0);
    }
    SECTION("rvalue engaged") {
        int  count = 0;
        auto f     = [&] {
            ++count;
            return 0;
        };
        CHECK(fvo::or_invoke(std::optional<int>{42}, f) == 42);
        CHECK(count == 0);
    }
    SECTION("lvalue disengaged") {
        std::optional<int> m{};
        int                count = 0;
        CHECK(fvo::or_invoke(m, [&] {
                  ++count;
                  return 99;
              }) == 99);
        CHECK(count == 1);
    }
    SECTION("rvalue disengaged") {
        int count = 0;
        CHECK(fvo::or_invoke(std::optional<int>{}, [&] {
                  ++count;
                  return 99;
              }) == 99);
        CHECK(count == 1);
    }
}

// ==========================================================================
// Different return type: invocable returns long
// ==========================================================================

TEST_CASE("or_invoke: invocable returning long promotes result to long", "[or_invoke]") {
    std::optional<int> engaged{42};
    std::optional<int> disengaged{};

    auto f = [] { return 99L; };

    static_assert(std::is_same_v<decltype(fvo::or_invoke(engaged, f)), long>);

    CHECK(fvo::or_invoke(engaged, f) == 42L);
    CHECK(fvo::or_invoke(disengaged, f) == 99L);
}

// ==========================================================================
// Reference-returning invocable: R decays to value (common_type, not common_reference)
// ==========================================================================

TEST_CASE("or_invoke: reference-returning invocable yields value type", "[or_invoke]") {
    static int fallback_val = 99;
    // invoke_result_t of this lambda is int&, but common_type_t<int&, int&> = int
    auto ref_f = []() -> int& { return fallback_val; };

    static_assert(std::is_same_v<decltype(fvo::or_invoke(std::declval<std::optional<int>&>(), ref_f)), int>);

    std::optional<int> engaged{42};
    std::optional<int> disengaged{};

    CHECK(fvo::or_invoke(engaged, ref_f) == 42);
    CHECK(fvo::or_invoke(disengaged, ref_f) == 99);
    // Result is a copy (int), not a reference — modifying fallback_val after the
    // call would not affect a stored result (value semantics, unlike reference_or).
}

// ==========================================================================
// Stateful / move-only invocable: I&& forwarding
// ==========================================================================

TEST_CASE("or_invoke: mutable lambda (stateful invocable)", "[or_invoke]") {
    // Mutable lambda has non-const operator(); or_invoke must forward I&& correctly.
    int  internal  = 0;
    auto mutable_f = [internal]() mutable { return ++internal; };

    std::optional<int> disengaged{};
    CHECK(fvo::or_invoke(disengaged, mutable_f) == 1);
}

TEST_CASE("or_invoke: move-only invocable (unique_ptr capture)", "[or_invoke]") {
    // or_invoke must not copy the invocable; it must forward I&& to invocable().
    auto ptr = std::make_unique<int>(55);
    auto f   = [p = std::move(ptr)]() -> int { return *p; };

    std::optional<int> engaged{42};
    std::optional<int> disengaged{};

    // Engaged: invocable NOT called → the unique_ptr inside f stays valid.
    CHECK(fvo::or_invoke(engaged, std::move(f)) == 42);
}

TEST_CASE("or_invoke: move-only invocable disengaged path", "[or_invoke]") {
    auto ptr = std::make_unique<int>(55);
    auto f   = [p = std::move(ptr)]() -> int { return *p; };

    std::optional<int> disengaged{};
    CHECK(fvo::or_invoke(disengaged, std::move(f)) == 55);
}
