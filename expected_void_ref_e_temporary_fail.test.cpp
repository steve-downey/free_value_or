// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: cannot bind temporary to E& — dangling prevention
#include <beman/expected/expected.hpp>
void test() { beman::expected::expected<void, int&> e(beman::expected::unexpect, 42); }
