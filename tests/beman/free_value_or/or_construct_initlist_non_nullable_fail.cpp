// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Must NOT compile: std::string does not satisfy nullable.
// The braced init-list forces the init-list overload; the only reason to reject is the nullable constraint.

#include <beman/free_value_or/value_or.hpp>
#include <string>

void check() {
    (void)smd::free_value_or::or_construct(std::string{"a"}, {1, 2, 3});
}
