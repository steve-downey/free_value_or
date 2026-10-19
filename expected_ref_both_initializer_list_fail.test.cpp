// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: expected<T&, E&> initializer-list error construction: E& cannot bind to the list
// EXPECT: "initializer-list error construction cannot bind a reference|use of deleted function|call to deleted"
#include <beman/expected/expected.hpp>

using namespace beman::expected;

expected<int&, const int&> value(unexpect, {1, 2, 3});
