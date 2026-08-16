// tests/beman/optional/value_or_else_bad_conversion_fail.test.cpp      -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/optional/optional.hpp>

#include <string>

void test() {
    beman::optional::optional<int> empty;
    (void)empty.value_or_else([] { return std::string{"not an int"}; });
}
