// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE COMPILE TEST: unexpected<unexpected<int>> is ill-formed [expected.un.general] para 2
// E must not be a specialization of unexpected.
#include <beman/expected/unexpected.hpp>

using namespace beman::expected;
unexpected<unexpected<int>> u(unexpected<int>(42)); // must not compile
