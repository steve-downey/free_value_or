// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE COMPILE TEST: expected<int, int&> is ill-formed [expected.object.general] para 2
// E must not be a reference type.
#include <beman/expected/expected.hpp>

beman::expected::expected<int, int&> e; // must not compile
