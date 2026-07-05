// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Negative compile test: binding a temporary to E& in expected<T&, E&> must fail.
#include <beman/expected/expected.hpp>

using namespace beman::expected;

void test() {
    expected<int&, int&> e(unexpect, 42); // binds temporary int to E& — must not compile
}
