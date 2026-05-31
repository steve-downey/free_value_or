// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE COMPILE TEST: unexpected<int[]> is ill-formed [expected.un.general] para 2
// E must not be an array type.
#include <beman/expected/unexpected.hpp>

beman::expected::unexpected<int[]> u(std::in_place); // must not compile
