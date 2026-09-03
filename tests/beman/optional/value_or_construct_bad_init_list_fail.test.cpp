// tests/beman/optional/value_or_construct_bad_init_list_fail.test.cpp  -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/optional/optional.hpp>

#include <vector>

void test() {
    beman::optional::optional<std::vector<int>> empty;
    (void)empty.value_or_construct({"not an int"});
}
