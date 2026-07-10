// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: expected<int[3], int> is ill-formed [expected.object.general] para 2
// EXPECT: "T must not be an array type"
#include <beman/expected/expected.hpp>

beman::expected::expected<int[3], int> e; // must not compile
