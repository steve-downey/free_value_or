// tests/beman/expected/expected_ref_temporary_fail.cpp               -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: constructing expected<int&, E> from a temporary must not compile
// EXPECT: "no matching function|delete"
#include <beman/expected/expected.hpp>
void test() {
    beman::expected::expected<int&, int> e(42); // 42 is a temporary
}
