// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Negative compile test: expected<void, E[]> where E is an array is ill-formed.
#include <beman/expected/expected.hpp>

beman::expected::expected<void, int[]> x; // should fail
