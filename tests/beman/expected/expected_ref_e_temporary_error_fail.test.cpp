// tests/beman/expected/expected_ref_e_temporary_error_fail.cpp        -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: cannot bind a temporary (rvalue) to E& — dangling prevention
// EXPECT: "delete|no matching function"
#include <beman/expected/expected.hpp>
void test() {
    // 42 is a temporary — expected<int, int&> must refuse to bind E& to it
    beman::expected::expected<int, int&> e(beman::expected::unexpect, 42);
}
