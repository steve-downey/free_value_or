// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Must NOT compile: the fallback arguments cannot construct the result type.

#include <beman/free_value_or/value_or.hpp>

#include <optional>
#include <string>

void check() {
    std::optional<std::string> empty;
    (void)smd::free_value_or::or_construct<std::string>(empty, 42);
}
