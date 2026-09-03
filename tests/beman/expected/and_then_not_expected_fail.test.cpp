// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: and_then Mandates U is a specialization of expected
// EXPECT: "F must return a specialization of expected"
#include <beman/expected/expected.hpp>
void test() {
    beman::expected::expected<int, int> e(1);
    e.and_then([](int v) -> int { return v; });
}
