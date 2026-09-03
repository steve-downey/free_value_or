// tests/beman/expected/value_or_construct_bad_init_list_fail.test.cpp  -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/expected/expected.hpp>

#include <vector>

void test() {
    beman::expected::expected<std::vector<int>, int> error = beman::expected::unexpected(0);
    (void)error.value_or_construct({"not an int"});
}
