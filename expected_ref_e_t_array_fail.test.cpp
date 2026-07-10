// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: T must not be an array type in expected<T, E&>
// EXPECT: "T must not be an array type"
#include <beman/expected/expected.hpp>
void test() { beman::expected::expected<int[3], int&> e{}; }
