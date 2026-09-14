// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/free_value_or/value_or.hpp>

#if defined(BEMAN_OPTIONAL_OPTIONAL_HPP)
    #error "the core free_value_or header must not include beman::optional"
#endif

#if defined(BEMAN_EXPECTED_EXPECTED_HPP)
    #error "the core free_value_or header must not include beman::expected"
#endif

#include <beman/expected/expected.hpp>
#include <beman/optional/optional.hpp>

#include <catch2/catch_test_macros.hpp>

static_assert(!smd::free_value_or::enable_borrowed_nullable<beman::optional::optional<int&>>);

#if defined(BEMAN_EXPECTED_HAS_REFERENCES) && BEMAN_EXPECTED_HAS_REFERENCES
static_assert(!smd::free_value_or::enable_borrowed_nullable<beman::expected::expected<int&, int>>);
#endif

TEST_CASE("core header does not implicitly integrate vendored nullables", "[reference_or][integration]") { SUCCEED(); }
