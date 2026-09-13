// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Must NOT compile: the initializer list cannot construct the result type.

#include <beman/free_value_or/value_or.hpp>

#include <optional>
#include <string>
#include <vector>

void check() {
    std::optional<std::vector<int>> empty;
    (void)smd::free_value_or::or_construct<std::vector<int>>(empty, {std::string{"x"}});
}
