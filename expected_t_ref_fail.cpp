// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: expected<int&, int> now matches the reference specialization which has no default ctor
// EXPECT: "delete"
#include <beman/expected/expected.hpp>

beman::expected::expected<int&, int> e; // must not compile
