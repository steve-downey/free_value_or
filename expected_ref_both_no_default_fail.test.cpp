// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Negative compile test: expected<T&, E&> has no default constructor.
#include <beman/expected/expected.hpp>

using namespace beman::expected;

void test() {
    expected<int&, int&> e; // no default constructor — must not compile
}
