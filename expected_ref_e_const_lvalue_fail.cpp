// tests/beman/expected/expected_ref_e_const_lvalue_fail.cpp           -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: cannot bind a const lvalue to non-const E& — language-level constraint
// EXPECT: "no matching function|cannot bind"
#include <beman/expected/expected.hpp>
void test() {
    const int err = 5;
    // expected<int, int&> (non-const E) cannot be constructed from const int lvalue
    beman::expected::expected<int, int&> e(beman::expected::unexpect, err);
}
