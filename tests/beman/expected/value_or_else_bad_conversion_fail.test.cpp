// tests/beman/expected/value_or_else_bad_conversion_fail.test.cpp      -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// EXPECT: "value_or_else requires is_convertible_v<invoke_result_t<F>, T>"

#include <beman/expected/expected.hpp>

#include <string>

void test() {
    beman::expected::expected<int, int> error = beman::expected::unexpected(0);
    (void)error.value_or_else([] { return std::string{"not an int"}; });
}
