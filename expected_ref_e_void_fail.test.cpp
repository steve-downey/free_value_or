// tests/beman/expected/expected_ref_e_void_fail.cpp                  -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: expected<T&, E> where E is void is ill-formed
// EXPECT: "E must not be void"
#include <beman/expected/expected.hpp>
void test() {
    int                                   x = 0;
    beman::expected::expected<int&, void> e(x); // must not compile
}
