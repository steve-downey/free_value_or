// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: and_then on expected<T&,E> must return expected with same error_type
// EXPECT: "F must return expected with the same error_type"
#include <beman/expected/expected.hpp>
#include <string>
void test() {
    int                                  x = 0;
    beman::expected::expected<int&, int> e(x);
    // F returns expected<int&, std::string> but error_type must be int
    e.and_then([](int& v) -> beman::expected::expected<int&, std::string> {
        return beman::expected::unexpected<std::string>("err");
    });
}
