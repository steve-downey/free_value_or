// tests/beman/expected/expected_ref_e_array_fail.cpp                 -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: expected<T&, E> where E is an array type is ill-formed
// EXPECT: "E must not be an array type"
#include <beman/expected/expected.hpp>
void test() {
    int                                     x = 0;
    beman::expected::expected<int&, int[5]> e(x); // must not compile
}
