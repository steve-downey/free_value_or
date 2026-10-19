// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: expected<T&, E&> has no in_place_t value constructor
// EXPECT: "no in_place value constructor|use of deleted function|call to deleted|attempting to reference"
#include <beman/expected/expected.hpp>
#include <utility>
void test() {
    int                                   err = 0;
    beman::expected::expected<int&, int&> e(std::in_place, 42);
}
