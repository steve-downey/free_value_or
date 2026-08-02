// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: cannot bind const lvalue to non-const E&
#include <beman/expected/expected.hpp>
void test() {
    const int                             err = 5;
    beman::expected::expected<void, int&> e(beman::expected::unexpect, err);
}
