// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: transform_error on expected<T,E&> must return object type G, not reference
// EXPECT: "G must be an object type"
#include <beman/expected/expected.hpp>
void test() {
    int                                  err = 0;
    beman::expected::expected<int, int&> e(beman::expected::unexpect, err);
    // G = int& (reference type, not object type)
    e.transform_error([](int& v) -> int& { return v; });
}
