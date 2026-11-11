// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: expected<T&, E&> has no default constructor
// EXPECT: "no default constructor|use of deleted function|call to deleted|attempting to reference"
#include <beman/expected/expected.hpp>

using namespace beman::expected;

void test() {
    expected<int&, int&> e; // no default constructor — must not compile
}
