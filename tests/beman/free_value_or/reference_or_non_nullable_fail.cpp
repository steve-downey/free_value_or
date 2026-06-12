// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Must NOT compile: std::string does not satisfy nullable, so reference_or must be rejected.

#include <beman/free_value_or/value_or.hpp>
#include <string>

void check() {
    std::string a{"a"}, b{"b"};
    (void)smd::free_value_or::reference_or(a, b);
}
