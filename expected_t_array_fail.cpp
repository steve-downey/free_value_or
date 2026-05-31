// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE COMPILE TEST: expected<int[3], int> is ill-formed [expected.object.general] para 2
// T must not be an array type.
#include <beman/expected/expected.hpp>

beman::expected::expected<int[3], int> e; // must not compile
