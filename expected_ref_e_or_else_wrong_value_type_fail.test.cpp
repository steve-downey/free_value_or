// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: or_else on expected<T,E&> must return expected with same value_type (T)
// EXPECT: "F must return expected with the same value_type"
#include <beman/expected/expected.hpp>
void test() {
    int                                  err = 0;
    beman::expected::expected<int, int&> e(beman::expected::unexpect, err);
    // F returns expected<double, int&> but value_type must be int
    e.or_else([](int& v) -> beman::expected::expected<double, int&> { return 0.0; });
}
