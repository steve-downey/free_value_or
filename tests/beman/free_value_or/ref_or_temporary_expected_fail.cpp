// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Must NOT compile: destroying a temporary owning expected destroys the
// object to which reference_or would return a reference.

#include <beman/free_value_or/value_or.hpp>

#include <expected>

void check() {
    int fallback = 0;
    (void)smd::free_value_or::reference_or(std::expected<int, int>{42}, fallback);
}
