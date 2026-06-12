// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Must NOT compile: std::vector<int> does not satisfy nullable, so or_invoke must be rejected.

#include <beman/free_value_or/value_or.hpp>
#include <vector>

void check() {
    std::vector<int> v{};
    (void)smd::free_value_or::or_invoke(v, [] { return 0; });
}
