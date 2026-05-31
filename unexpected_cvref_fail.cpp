// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE COMPILE TEST: unexpected<const int> is ill-formed [expected.un.general] para 2
// E must not be cv-qualified.
#include <beman/expected/unexpected.hpp>

beman::expected::unexpected<const int> u(42); // must not compile
