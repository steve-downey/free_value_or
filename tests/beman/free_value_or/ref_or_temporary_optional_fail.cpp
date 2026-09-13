// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Must NOT compile: destroying a temporary owning optional destroys the
// object to which reference_or would return a reference.

#include <beman/free_value_or/value_or.hpp>

#include <optional>

void check() {
    int fallback = 0;
    (void)smd::free_value_or::reference_or(std::optional<int>{42}, fallback);
}
