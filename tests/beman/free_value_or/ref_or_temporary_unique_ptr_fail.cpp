// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Must NOT compile: a temporary unique_ptr normally destroys its referent.

#include <beman/free_value_or/value_or.hpp>

#include <memory>

void check() {
    int fallback = 0;
    (void)smd::free_value_or::reference_or(std::make_unique<int>(42), fallback);
}
