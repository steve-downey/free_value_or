// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: and_then Mandates U::error_type == E
// F returns expected<int, double> but E is std::string — ill-formed
#include <beman/expected/expected.hpp>
#include <string>
void test() {
    beman::expected::expected<int, std::string> e(1);
    e.and_then([](int v) -> beman::expected::expected<int, double> { return v; });
}
