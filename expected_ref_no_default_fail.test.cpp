// tests/beman/expected/expected_ref_no_default_fail.cpp              -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: expected<T&, E> has no default constructor
// EXPECT: "no default constructor|use of deleted function|call to deleted|attempting to reference"
#include <beman/expected/expected.hpp>
void test() {
    beman::expected::expected<int&, int> e; // must not compile
}
