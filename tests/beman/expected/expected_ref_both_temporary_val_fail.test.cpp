// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: binding a temporary to T& in expected<T&, E&> must fail
// EXPECT: "binding a temporary to T& creates a dangling reference|use of deleted function|call to deleted"
// EXPECT: "no matching function|no matching constructor|no overloaded function could convert|attempting to reference"
#include <beman/expected/expected.hpp>

using namespace beman::expected;

void test() {
    expected<int&, int&> e(42); // binds temporary int to T& — must not compile
}
