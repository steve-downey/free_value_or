// tests/beman/expected/value_or_else_bad_conversion_fail.test.cpp      -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/expected/expected.hpp>

#include <string>

void test() {
    beman::expected::expected<int, int> error = beman::expected::unexpected(0);
    (void)error.value_or_else([] { return std::string{"not an int"}; });
}
