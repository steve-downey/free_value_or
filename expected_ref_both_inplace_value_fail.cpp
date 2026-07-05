// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: expected<T&, E&> has no in_place_t value constructor
// EXPECT: "no matching function"
#include <beman/expected/expected.hpp>
#include <utility>
void test() {
    int                                   err = 0;
    beman::expected::expected<int&, int&> e(std::in_place, 42);
}
