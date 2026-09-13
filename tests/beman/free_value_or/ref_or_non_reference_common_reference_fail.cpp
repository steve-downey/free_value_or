// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Must NOT compile: common_reference_t<int&, long&> is a value type. A
// reference-named operation must not silently materialize and return it.

#include <beman/free_value_or/value_or.hpp>

#include <optional>

void check() {
    std::optional<int> value{42};
    long               fallback = 0;
    (void)smd::free_value_or::reference_or(value, fallback);
}
