// NEGATIVE: or_else on void expected mandates G::value_type == void
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include <beman/expected/expected.hpp>
void test() {
    beman::expected::expected<void, int> e(beman::expected::unexpect, 1);
    e.or_else([](int) -> beman::expected::expected<int, int> { return 0; });
}
