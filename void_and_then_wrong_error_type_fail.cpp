// NEGATIVE: and_then on void expected mandates U::error_type == E
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include <beman/expected/expected.hpp>
void test() {
    beman::expected::expected<void, int> e;
    e.and_then([]() -> beman::expected::expected<int, double> { return 0; });
}
