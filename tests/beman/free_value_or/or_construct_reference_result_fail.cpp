// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Must NOT compile: a reference result can bind to the temporary constructed
// from the fallback arguments and dangle on return.

#include <beman/free_value_or/value_or.hpp>

#include <optional>

void check() {
    std::optional<int> empty;
    (void)smd::free_value_or::or_construct<const int&>(empty, 42);
}
