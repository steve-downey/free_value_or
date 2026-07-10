// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: expected<void, E[]> where E is an array is ill-formed
// EXPECT: "E must not be an array type"
#include <beman/expected/expected.hpp>

beman::expected::expected<void, int[]> x; // should fail
