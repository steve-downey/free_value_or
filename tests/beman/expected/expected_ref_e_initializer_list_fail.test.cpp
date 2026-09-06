// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include <beman/expected/expected.hpp>

using namespace beman::expected;

expected<int, const int&> value(unexpect, {1, 2, 3});
