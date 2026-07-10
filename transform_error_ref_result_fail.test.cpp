// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: transform_error Mandates G is valid for unexpected (not a reference)
// EXPECT: "G must be an object type"
#include <beman/expected/expected.hpp>
void test() {
    beman::expected::expected<int, int> e(beman::expected::unexpect, 1);
    e.transform_error([](int) -> int& {
        static int x = 0;
        return x;
    });
}
