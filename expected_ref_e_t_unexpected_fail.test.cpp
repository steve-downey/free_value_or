// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: T must not be a specialization of unexpected in expected<T, E&>
// EXPECT: "T must not be a specialization of unexpected"
#include <beman/expected/expected.hpp>
void test() { beman::expected::expected<beman::expected::unexpected<int>, int&> e{}; }
