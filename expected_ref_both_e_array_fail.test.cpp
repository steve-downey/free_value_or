// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: E must not be an array type in expected<T&, E&>
// EXPECT: "E must not be an array type"
#include <beman/expected/expected.hpp>
void test() {
    using arr                               = int[3];
    int                                   x = 0;
    beman::expected::expected<int&, arr&> e(x);
}
