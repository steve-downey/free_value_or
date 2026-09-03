// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: T must not be an array type in expected<T&, E&>
// EXPECT: "T must not be an array type"
#include <beman/expected/expected.hpp>
void test() {
    using arr                                 = int[3];
    int                                   err = 0;
    beman::expected::expected<arr&, int&> e(beman::expected::unexpect, err);
}
