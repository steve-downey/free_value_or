// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: expected<void, E&> has no value_or member
#include <beman/expected/expected.hpp>
void test() {
    beman::expected::expected<void, int&> e;
    e.value_or(0);
}
