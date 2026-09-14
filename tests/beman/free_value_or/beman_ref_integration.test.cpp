// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/free_value_or/free_value_or.hpp>
#include <beman/free_value_or/beman_expected.hpp>
#include <beman/free_value_or/beman_optional.hpp>

#include <catch2/catch_test_macros.hpp>

static_assert(smd::free_value_or::borrowed_nullable<beman::optional::optional<int&>>);

#if defined(BEMAN_EXPECTED_HAS_REFERENCES) && BEMAN_EXPECTED_HAS_REFERENCES
static_assert(smd::free_value_or::borrowed_nullable<beman::expected::expected<int&, int>>);
#endif

TEST_CASE("explicit integration headers opt in vendored reference nullables", "[reference_or][integration]") {
    int value    = 42;
    int fallback = 0;

    int& optional_result = smd::free_value_or::reference_or(beman::optional::optional<int&>{value}, fallback);
    CHECK(&optional_result == &value);

#if defined(BEMAN_EXPECTED_HAS_REFERENCES) && BEMAN_EXPECTED_HAS_REFERENCES
    int& expected_result = smd::free_value_or::reference_or(beman::expected::expected<int&, int>{value}, fallback);
    CHECK(&expected_result == &value);
#endif
}
