// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Include-guard idempotency check (matching existing convention)
#include <beman/free_value_or/value_or.hpp>
#include <beman/free_value_or/value_or.hpp>

#include "test_types.hpp"

#include <catch2/catch_test_macros.hpp>

#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <vector>

// ==========================================================================
// Positive: types that MUST satisfy nullable
// ==========================================================================

static_assert(fvo::nullable<std::optional<int>>);
static_assert(fvo::nullable<std::optional<std::string>>);

// const-qualified form (concept checks 'const T' internally, but the outer
// type being const should still satisfy it)
static_assert(fvo::nullable<const std::optional<int>>);

#if FVO_HAS_STD_EXPECTED
// std::expected<T,E>: bool(e) and *e are both valid on a const expected
static_assert(fvo::nullable<std::expected<int, int>>);
static_assert(fvo::nullable<std::expected<std::string, int>>);
#endif // FVO_HAS_STD_EXPECTED

// Raw pointers
static_assert(fvo::nullable<int*>);
static_assert(fvo::nullable<const int*>);
static_assert(fvo::nullable<double*>);

// Smart pointers
static_assert(fvo::nullable<std::shared_ptr<int>>);
static_assert(fvo::nullable<std::unique_ptr<int>>);

// optional<T&> via vendored beman::optional (gated on FVO_HAS_OPTIONAL_REF)
#if FVO_HAS_OPTIONAL_REF
static_assert(fvo::nullable<fvo_opt::optional<int&>>);
static_assert(fvo::nullable<fvo_opt::optional<const int&>>);
#endif

// ==========================================================================
// Negative: types that must NOT satisfy nullable
// ==========================================================================

// Plain scalars: contextual-bool but no operator*
static_assert(!fvo::nullable<int>);
static_assert(!fvo::nullable<double>);
static_assert(!fvo::nullable<bool>);

// Standard containers/strings: neither bool nor deref-to-value
static_assert(!fvo::nullable<std::string>);
static_assert(!fvo::nullable<std::vector<int>>);

// Anti-model types from test_types.hpp
static_assert(!fvo::nullable<bool_only>);         // operator bool, no operator*
static_assert(!fvo::nullable<deref_only>);        // operator*, no contextual bool
static_assert(!fvo::nullable<nonconst_nullable>); // both operators are non-const

// void and nullptr_t: no operator bool / no operator*
static_assert(!fvo::nullable<void>);
static_assert(!fvo::nullable<std::nullptr_t>);

// ==========================================================================
// Runtime TEST_CASE so the exe links and ctest sees a result
// ==========================================================================

TEST_CASE("nullable concept: static assertions hold at compile time") {
    // All real coverage is above via static_assert.
    // Spot-check one runtime fact: optional<int> satisfies nullable.
    std::optional<int> engaged{42};
    std::optional<int> empty{};
    REQUIRE(fvo::nullable<decltype(engaged)>);
    REQUIRE(fvo::nullable<decltype(empty)>);
    REQUIRE(!fvo::nullable<int>);
}
