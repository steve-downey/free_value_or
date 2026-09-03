// tests/beman/expected/value_or_construct_bad_args_fail.test.cpp       -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/expected/expected.hpp>

#include <string>

void test() {
    beman::expected::expected<std::string, int> error = beman::expected::unexpected(0);
    (void)error.value_or_construct(42);
}
