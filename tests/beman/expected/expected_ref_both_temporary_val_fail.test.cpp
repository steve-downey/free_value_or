// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Negative compile test: binding a temporary to T& in expected<T&, E&> must fail.
#include <beman/expected/expected.hpp>

using namespace beman::expected;

void test() {
    expected<int&, int&> e(42); // binds temporary int to T& — must not compile
}
