// tests/beman/expected/expected_ref_temporary_fail.cpp               -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: constructing expected<int&, E> from a temporary must not compile
// EXPECT: "binding a temporary to T& creates a dangling reference|use of deleted function|call to deleted"
// EXPECT: "no matching function|no matching constructor|no overloaded function could convert|attempting to reference"
#include <beman/expected/expected.hpp>
void test() {
    beman::expected::expected<int&, int> e(42); // 42 is a temporary
}
