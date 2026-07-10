// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: T must not be in_place_t in expected<T, E&>
// EXPECT: "T must not be in_place_t"
#include <beman/expected/expected.hpp>
#include <utility>
void test() { beman::expected::expected<std::in_place_t, int&> e{}; }
