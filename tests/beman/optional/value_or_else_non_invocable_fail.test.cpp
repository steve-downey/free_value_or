// tests/beman/optional/value_or_else_non_invocable_fail.test.cpp       -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/optional/optional.hpp>

struct non_invocable {};

void test() {
    beman::optional::optional<int> empty;
    (void)empty.value_or_else(non_invocable{});
}
