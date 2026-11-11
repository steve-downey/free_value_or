// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: binding a temporary to E& in expected<T&, E&> must fail
// EXPECT: "cannot bind to non-const E&|use of deleted function|call to deleted|attempting to reference"
#include <beman/expected/expected.hpp>

using namespace beman::expected;

void test() {
    expected<int&, int&> e(unexpect, 42); // binds temporary int to E& — must not compile
}
