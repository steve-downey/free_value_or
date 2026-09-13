// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Must NOT compile: a temporary shared_ptr may be the last owner and destroy
// its referent at the end of the full-expression.

#include <beman/free_value_or/value_or.hpp>

#include <memory>

void check() {
    int fallback = 0;
    (void)smd::free_value_or::reference_or(std::make_shared<int>(42), fallback);
}
