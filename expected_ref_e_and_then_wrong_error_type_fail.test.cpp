// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: and_then on expected<T,E&> must return expected with same error_type (E&)
// EXPECT: "F must return expected with the same error_type"
#include <beman/expected/expected.hpp>
void test() {
    int                                  err = 0;
    beman::expected::expected<int, int&> e(42);
    // F returns expected<int, int> but error_type must be int&
    e.and_then([](int v) -> beman::expected::expected<int, int> { return v; });
}
