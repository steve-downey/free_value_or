// tests/beman/expected/expected_ref_inplace_value_fail.cpp           -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: expected<T&, E> has no in_place_t value constructor
// EXPECT: "no matching function"
#include <beman/expected/expected.hpp>
void test() {
    int                                  x = 5;
    beman::expected::expected<int&, int> e(std::in_place, x); // must not compile
}
