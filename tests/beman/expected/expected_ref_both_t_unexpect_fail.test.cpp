// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: T must not be unexpect_t in expected<T&, E&>
// EXPECT: "T must not be unexpect_t"
#include <beman/expected/expected.hpp>
void test() {
    beman::expected::unexpect_t                                   u{};
    int                                                           err = 0;
    beman::expected::expected<beman::expected::unexpect_t&, int&> e(u);
}
