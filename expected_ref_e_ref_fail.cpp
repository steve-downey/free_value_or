// tests/beman/expected/expected_ref_e_ref_fail.cpp                   -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: expected<T&, E&&> — rvalue reference as E in expected<T&,E> must fail
// EXPECT: "E must not be an rvalue reference"
#include <beman/expected/expected.hpp>
void test() {
    int                                    x = 0;
    beman::expected::expected<int&, int&&> e(x); // E is rvalue ref — must not compile
}
